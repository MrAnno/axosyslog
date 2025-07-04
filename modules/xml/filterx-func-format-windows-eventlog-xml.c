/*
 * Copyright (c) 2025 Axoflow
 * Copyright (c) 2025 Tamas Kosztyu <tamas.kosztyu@axoflow.com>
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
#include "filterx-func-format-windows-eventlog-xml.h"


FilterXExpr *
filterx_function_format_windows_eventlog_xml_new(FilterXFunctionArgs *args, GError **error)
{
  FilterXExpr *s = filterx_function_format_xml_new(args, error);
  FilterXFunctionFormatXML *self = (FilterXFunctionFormatXML *)s;

  if (!self)
    return NULL;

  return s;
}

FILTERX_FUNCTION(format_windows_eventlog_xml, filterx_function_format_windows_eventlog_xml_new);
