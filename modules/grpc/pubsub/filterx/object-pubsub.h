/*
 * Copyright (c) 2025 Axoflow
 * Copyright (c) 2025 shifter
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

#ifndef OBJECT_PUBSUB_H
#define OBJECT_PUBSUB_H

#include "syslog-ng.h"

#include "compat/cpp-start.h"
#include "filterx/filterx-object.h"
#include "filterx/expr-function.h"

FILTERX_SIMPLE_FUNCTION_DECLARE(pubsub_message);

FilterXObject *filterx_pubsub_message_new_from_args(FilterXExpr *s, FilterXObject *args[], gsize args_len);

static inline FilterXObject *
filterx_pubsub_message_new(void)
{
  return filterx_pubsub_message_new_from_args(NULL, NULL, 0);
}

FILTERX_DECLARE_TYPE(pubsub_message);

static inline void
pubsub_filterx_objects_global_init(void)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      filterx_type_init(&FILTERX_TYPE_NAME(pubsub_message));
      initialized = TRUE;
    }
}

#include "compat/cpp-end.h"

#endif
