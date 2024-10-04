/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2024 Szilard Parrag
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#ifndef FILTERX_FUNC_SDATA_H_INCLUDED
#define FILTERX_FUNC_SDATA_H_INCLUDED

#include "filterx/expr-function.h"

FilterXExpr *filterx_function_is_sdata_from_enterprise_new(FilterXFunctionArgs *args, GError **error);
FilterXObject *filterx_simple_function_has_sdata(FilterXExpr *s, GPtrArray *args);
FilterXExpr *filterx_generator_function_get_sdata_new(FilterXFunctionArgs *args, GError **error);
typedef struct FilterXGenFuncGetSdata_
{
  FilterXGeneratorFunction super;
} FilterXGenFuncGetSdata;

#endif