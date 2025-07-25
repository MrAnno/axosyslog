/*
 * Copyright (c) 2002-2017 Balabit
 * Copyright (c) 2017 Laszlo Budai
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

#ifndef STATS_CLUSTER_LOGPIPE_H_INCLUDED
#define STATS_CLUSTER_LOGPIPE_H_INCLUDED

#include "syslog-ng.h"

typedef enum
{
  SC_TYPE_DROPPED=0, /* number of messages dropped */
  SC_TYPE_PROCESSED, /* number of messages processed */
  SC_TYPE_QUEUED,    /* number of messages on disk */
  SC_TYPE_SUPPRESSED,/* number of messages suppressed */
  SC_TYPE_STAMP,     /* timestamp */
  SC_TYPE_DISCARDED, /* discarded messages of filter */
  SC_TYPE_MATCHED, /* discarded messages of filter */
  SC_TYPE_NOT_MATCHED, /* discarded messages of filter */
  SC_TYPE_WRITTEN, /* number of sent messages */
  SC_TYPE_MAX
} StatsCounterGroupLogPipe;

void stats_cluster_logpipe_key_set(StatsClusterKey *key, const gchar *name,
                                   StatsClusterLabel *labels, gsize labels_len);


/*
 * The legacy functions should not be used for new code.
 * When transforming a legacy stats key to a new metric key, aliases should be specified consistently at all call sites.
 */
void stats_cluster_logpipe_key_legacy_set(StatsClusterKey *key, guint16 component, const gchar *id,
                                          const gchar *instance);
void stats_cluster_logpipe_key_add_legacy_alias(StatsClusterKey *key, guint16 component, const gchar *id,
                                                const gchar *instance);
#endif
