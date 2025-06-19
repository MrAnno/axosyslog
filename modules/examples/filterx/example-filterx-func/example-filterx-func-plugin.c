/*
 * Copyright (c) 2023 shifter
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "cfg-parser.h"
#include "plugin.h"
#include "plugin-types.h"
#include "filterx/filterx-object.h"
#include "scratch-buffers.h"
#include "example-filterx-func-plugin.h"
#include "filterx/object-primitive.h"

static FilterXObject *
echo(FilterXExpr *s, FilterXObject *args[], gsize args_len)
{
  GString *buf = scratch_buffers_alloc();
  LogMessageValueType t;

  if (args == NULL ||
      args_len < 1)
    {
      return NULL;
    }

  for (int i = 0; i < args_len; i++)
    {
      if (!filterx_object_marshal(args[i], buf, &t))
        goto exit;
      msg_debug("FILTERX EXAMPLE ECHO",
                evt_tag_str("value", buf->str),
                evt_tag_str("type", log_msg_value_type_to_str(t)));
    }
  if (args_len > 0)
    return filterx_object_ref(args[0]);
exit:
  return filterx_boolean_new(FALSE);
}

FILTERX_SIMPLE_FUNCTION(example_echo, echo);
