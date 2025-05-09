/*
 * Copyright (c) 2023 Balazs Scheidler <balazs.scheidler@axoflow.com>
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
#ifndef FILTERX_MESSAGE_VALUE_H_INCLUDED
#define FILTERX_MESSAGE_VALUE_H_INCLUDED

#include "filterx/filterx-object.h"

FILTERX_DECLARE_TYPE(message_value);

FilterXObject *filterx_message_value_new_borrowed(const gchar *repr, gssize repr_len, LogMessageValueType type);
FilterXObject *filterx_message_value_new_ref(gchar *repr, gssize repr_len, LogMessageValueType type);
FilterXObject *filterx_message_value_new(const gchar *repr, gssize repr_len, LogMessageValueType type);

LogMessageValueType filterx_message_value_get_type(FilterXObject *s);
const gchar *filterx_message_value_get_value(FilterXObject *s, gsize *len);

gboolean filterx_message_value_get_string_ref(FilterXObject *s, const gchar **value, gsize *len);
gboolean filterx_message_value_get_bytes_ref(FilterXObject *s, const gchar **value, gsize *len);
gboolean filterx_message_value_get_protobuf_ref(FilterXObject *s, const gchar **value, gsize *len);
gboolean filterx_message_value_get_boolean(FilterXObject *s, gboolean *value);
gboolean filterx_message_value_get_integer(FilterXObject *s, gint64 *value);
gboolean filterx_message_value_get_double(FilterXObject *s, gdouble *value);
gboolean filterx_message_value_get_datetime(FilterXObject *s, UnixTime *value);
gboolean filterx_message_value_get_null(FilterXObject *s);
gboolean filterx_message_value_get_json(FilterXObject *s, struct json_object **value);

/* unmarshal a message representation into a FilterXObject */
FilterXObject *filterx_unmarshal_repr(const gchar *repr, gssize repr_len, LogMessageValueType t);

#endif
