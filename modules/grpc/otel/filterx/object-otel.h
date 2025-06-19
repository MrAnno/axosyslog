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

#ifndef OBJECT_OTEL_H
#define OBJECT_OTEL_H

#include "syslog-ng.h"

#include "compat/cpp-start.h"
#include "filterx/filterx-object.h"
#include "filterx/expr-function.h"

FILTERX_SIMPLE_FUNCTION_DECLARE(otel_logrecord);
FILTERX_SIMPLE_FUNCTION_DECLARE(otel_resource);
FILTERX_SIMPLE_FUNCTION_DECLARE(otel_scope);
FILTERX_SIMPLE_FUNCTION_DECLARE(otel_kvlist);
FILTERX_SIMPLE_FUNCTION_DECLARE(otel_array);


FilterXObject *filterx_otel_logrecord_new_from_args(FilterXExpr *s, FilterXObject *args[], gsize args_len);
FilterXObject *filterx_otel_resource_new_from_args(FilterXExpr *s, FilterXObject *args[], gsize args_len);
FilterXObject *filterx_otel_scope_new_from_args(FilterXExpr *s, FilterXObject *args[], gsize args_len);
FilterXObject *filterx_otel_kvlist_new_from_args(FilterXExpr *s, FilterXObject *args[], gsize args_len);
FilterXObject *filterx_otel_array_new_from_args(FilterXExpr *s, FilterXObject *args[], gsize args_len);

static inline FilterXObject *
filterx_otel_logrecord_new(void)
{
  return filterx_otel_logrecord_new_from_args(NULL, NULL, 0);
}

static inline FilterXObject *
filterx_otel_resource_new(void)
{
  return filterx_otel_resource_new_from_args(NULL, NULL, 0);
}

static inline FilterXObject *
filterx_otel_scope_new(void)
{
  return filterx_otel_scope_new_from_args(NULL, NULL, 0);
}

static inline FilterXObject *
filterx_otel_kvlist_new(void)
{
  return filterx_otel_kvlist_new_from_args(NULL, NULL, 0);
}

static inline FilterXObject *
filterx_otel_array_new(void)
{
  return filterx_otel_array_new_from_args(NULL, NULL, 0);
}

gpointer grpc_otel_filterx_enum_construct(Plugin *self);

FILTERX_DECLARE_TYPE(otel_logrecord);
FILTERX_DECLARE_TYPE(otel_resource);
FILTERX_DECLARE_TYPE(otel_scope);
FILTERX_DECLARE_TYPE(otel_kvlist);
FILTERX_DECLARE_TYPE(otel_array);

static inline void
otel_filterx_objects_global_init(void)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      filterx_type_init(&FILTERX_TYPE_NAME(otel_logrecord));
      filterx_type_init(&FILTERX_TYPE_NAME(otel_resource));
      filterx_type_init(&FILTERX_TYPE_NAME(otel_scope));
      filterx_type_init(&FILTERX_TYPE_NAME(otel_kvlist));
      filterx_type_init(&FILTERX_TYPE_NAME(otel_array));
      initialized = TRUE;
    }
}

#include "compat/cpp-end.h"

#endif
