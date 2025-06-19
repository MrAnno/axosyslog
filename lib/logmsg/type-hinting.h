/*
 * Copyright (c) 2012-2014 Balabit
 * Copyright (c) 2012-2014 Gergely Nagy <algernon@balabit.hu>
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

#ifndef TYPE_HINTING_H
#define TYPE_HINTING_H

#include "syslog-ng.h"
#include "logmsg/logmsg.h"
#include "compat/json.h"

#define TYPE_HINTING_ERROR type_hinting_error_quark()

GQuark type_hinting_error_quark(void);

enum TypeHintingError
{
  TYPE_HINTING_INVALID_TYPE,
  TYPE_HINTING_INVALID_CAST,
};

gboolean type_hint_parse(const gchar *hint, LogMessageValueType *out_hint, GError **error);

gboolean type_cast_drop_helper(gint drop_flags, const gchar *value, gssize value_len,
                               const gchar *type_hint);

gboolean type_cast_to_boolean(const gchar *value, gssize value_len, gboolean *out, GError **error);
gboolean type_cast_to_int32(const gchar *value, gssize value_len, gint32 *out, GError **error);
gboolean type_cast_to_int64(const gchar *value, gssize value_len, gint64 *out, GError **error);
gboolean type_cast_to_double(const gchar *value, gssize value_len, gdouble *out, GError **error);
gboolean type_cast_to_datetime_msec(const gchar *value, gssize value_len, gint64 *out, GError **error);
gboolean type_cast_to_datetime_unixtime(const gchar *value, gssize value_len, UnixTime *ut, GError **error);
gboolean type_cast_to_json(const gchar *value, gssize value_len, struct json_object **out, GError **error);
gboolean type_cast_to_json_from_list(const gchar *value, gssize value_len, struct json_object **out, GError **error);

gboolean type_cast_validate(const gchar *value, gssize value_len, LogMessageValueType type, GError **error);

#endif
