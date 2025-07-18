/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 1998-2013 Balázs Scheidler
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
#include "stats/stats-csv.h"
#include "stats/stats-registry.h"
#include "stats/stats-cluster.h"
#include "utf8utils.h"

#include <string.h>

static gboolean
has_csv_special_character(const gchar *var)
{
  if (strchr(var, ';'))
    return TRUE;
  if (strchr(var, '\"'))
    return TRUE;
  return FALSE;
}

static gchar *
stats_format_csv_escapevar(const gchar *var)
{
  gchar *escaped_result;

  if (var[0] && has_csv_special_character(var))
    {
      gchar *result;
      /* quote */
      result = convert_unsafe_utf8_to_escaped_binary(var, -1, AUTF8_UNSAFE_QUOTE);
      escaped_result = g_strdup_printf("\"%s\"", result);
      g_free(result);
    }
  else
    {
      escaped_result = convert_unsafe_utf8_to_escaped_binary(var, -1, 0);
    }
  return escaped_result;
}

static void
stats_format_csv(StatsCluster *sc, gint type, StatsCounterItem *counter, gpointer user_data)
{
  gpointer *args = (gpointer *) user_data;
  StatsCSVRecordFunc process_record = (StatsCSVRecordFunc) args[0];
  gpointer process_record_arg = args[1];
  gchar *s_id, *s_instance, *tag_name;
  gchar buf[32];
  gchar state;
  GString *csv = g_string_sized_new(512);

  s_id = stats_format_csv_escapevar(sc->key.legacy.id);
  s_instance = stats_format_csv_escapevar(sc->key.legacy.instance);

  if (sc->dynamic)
    state = 'd';
  else if (stats_cluster_is_orphaned(sc))
    state = 'o';
  else
    state = 'a';

  tag_name = stats_format_csv_escapevar(stats_cluster_get_type_name(sc, type));
  g_string_printf(csv, "%s;%s;%s;%c;%s;%"G_GSIZE_FORMAT"\n",
                  stats_cluster_get_component_name(sc, buf, sizeof(buf)),
                  s_id, s_instance, state, tag_name, stats_counter_get(&sc->counter_group.counters[type]));
  process_record(csv->str, process_record_arg);
  g_string_free(csv, TRUE);
  g_free(tag_name);
  g_free(s_id);
  g_free(s_instance);
}

void
stats_generate_csv(StatsCSVRecordFunc process_record, gpointer user_data, gboolean *cancelled)
{
  GString *csv = g_string_sized_new(512);

  g_string_printf(csv, "%s;%s;%s;%s;%s;%s\n", "SourceName", "SourceId", "SourceInstance", "State", "Type",
                  "Number");
  process_record(csv->str, user_data);
  g_string_free(csv, TRUE);
  gpointer format_csv_args[] = {process_record, user_data};
  stats_lock();
  stats_foreach_legacy_counter(stats_format_csv, format_csv_args, cancelled);
  stats_unlock();
}
