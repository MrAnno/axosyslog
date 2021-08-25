/*
 * Copyright (c) 2021 One Identity
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

#ifndef STATS_AGGREGATOR_H
#define STATS_AGGREGATOR_H

#include "stats/stats-counter.h"
#include "syslog-ng.h"
#include "stats-cluster.h"

typedef struct _StatsAggregator StatsAggregator;

#define HOUR_IN_SEC 3600 /* 60*60 */
#define DAY_IN_SEC 86400 /* 60*60*24 */

struct _StatsAggregator
{
  void (*insert_data)(StatsAggregator *self, gsize value);
  void (*aggregate)(StatsAggregator *self);
  void (*reset)(StatsAggregator *self);
  void (*free)(StatsAggregator *self);

  gssize use_count;
  StatsClusterKey key;
};

gboolean stats_aggregator_is_orphaned(StatsAggregator *self);

void stats_aggregator_track_counter(StatsAggregator *self);
void stats_aggregator_untrack_counter(StatsAggregator *self);

void stats_aggregator_insert_data(StatsAggregator *self, gsize value);
void stats_aggregator_aggregate(StatsAggregator *self);
void stats_aggregator_reset(StatsAggregator *self);
void stats_aggregator_free(StatsAggregator *self);

void stats_aggregator_item_init_instance(StatsAggregator *self, StatsClusterKey *sc_key);

StatsAggregator *stats_aggregat_maximum_new(gint level, StatsClusterKey *sc_key);

StatsAggregator *stats_aggregat_average_new(gint level, StatsClusterKey *sc_key);

StatsAggregator *stats_aggregat_cps_new(gint level, StatsClusterKey *sc_key, StatsCounterItem *counter);

#endif /* STATS_AGGREGATOR_H */