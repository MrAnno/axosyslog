/*
 * Copyright (c) 2002-2015 Balabit
 * Copyright (c) 1998-2015 Balázs Scheidler
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

#ifndef CSVPARSER_H_INCLUDED
#define CSVPARSER_H_INCLUDED

#include "parser/parser-expr.h"
#include "scanner/csv-scanner/csv-scanner.h"

typedef struct _CSVParserColumn
{
  gchar *name;
  LogMessageValueType type;
} CSVParserColumn;

CSVParserColumn *csv_parser_column_new(const gchar *name, LogMessageValueType type);
void csv_parser_column_free(CSVParserColumn *s);

CSVScannerOptions *csv_parser_get_scanner_options(LogParser *s);
void csv_parser_set_columns(LogParser *s, GList *columns);
gboolean csv_parser_set_flags(LogParser *s, guint32 flags);
void csv_parser_set_drop_invalid(LogParser *s, gboolean drop_invalid);
void csv_parser_set_prefix(LogParser *s, const gchar *prefix);
void csv_parser_set_list_name(LogParser *s, const gchar *list_name);
LogParser *csv_parser_new(GlobalConfig *cfg);
void csv_parser_set_on_error(LogParser *s, gint on_error);

guint32 csv_parser_lookup_flag(const gchar *flag);
gint csv_parser_lookup_dialect(const gchar *flag);


#endif
