/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 1998-2013 Balázs Scheidler
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

#include "stats/stats-control.h"
#include "stats/stats-log.h"
#include "stats/stats-query.h"
#include "stats/stats-registry.h"
#include "stats/stats-aggregator-registry.h"
#include "stats/stats.h"
#include "timeutils/cache.h"
#include "timeutils/misc.h"

#include <string.h>
#include <iv.h>

/*
 * The statistics module
 *
 * Various components of syslog-ng require counters to keep track of various
 * metrics, such as number of messages processed, dropped or stored in a
 * queue. For this purpose, this module provides an easy to use API to
 * register and keep track these counters, and also to publish them to
 * external programs via a UNIX domain socket.
 *
 * Counters are organized into clusters, as the same syslog-ng component
 * usually uses a set of related counters.  Each counter is typed (see
 * SC_TYPE_XXX enums), the most common is SC_TYPE_PROCESSED, which is merely
 * counts the number of messages being processed by a source/destination/etc.
 *
 * Each cluster has the following properties:
 *   * source component: enumerable type, that specifies the syslog-ng
 *     component that the given counter belongs to, examples:
 *       src.file, dst.file, center, src.socket, etc.
 *
 *   * id: the unique identifier of the syslog-ng configuration item that
 *     this counter belongs to. Named configuration elements (source,
 *     destination, etc) use their "name" here. Other components without a
 *     name use either an autogenerated ID (that can change when the
 *     configuration file changes), or an explicit ID configured by the
 *     administrator.
 *
 *   * instance: each configuration element may track several sets of
 *     counters. This field specifies an identifier that makes a group of
 *     counters unique. For instance:
 *      - source TCP drivers use the IP address of the client here
 *      - destination file writers use the expanded filename
 *      - for those which have no notion for instance, NULL is used
 *
 *   * state: dynamic, active or orphaned, this indicates whether the given
 *     counter is in use or in orphaned state
 *
 *   * type: counter type (processed, dropped, stored, etc)
 *
 * Threading
 *
 * Registration and unregistration must be protected expicitly by invoking
 * stats_lock()/unlock().  Once registered, counters can be manipulated
 * without acquiring stats_lock().
 *
 * Counters are updated atomically by the use of the stats_counter_inc/dec()
 * methods.
 */


static gboolean
stats_cluster_is_expired(StatsOptions *options, StatsCluster *sc, time_t now)
{
  time_t tstamp;

  /* check if dynamic entry, non-dynamic entries cannot be too large in
   * numbers, those are never pruned */
  if (!sc->dynamic)
    return FALSE;

  /* this entry is being updated, cannot be too old */
  if (sc->use_count > 0)
    return FALSE;

  /* check if timestamp is stored, no timestamp means we can't expire it.
   * All dynamic entries should have a timestamp.  */
  if ((sc->live_mask & (1 << SC_TYPE_STAMP)) == 0)
    return FALSE;

  tstamp = atomic_gssize_racy_get(&(sc->counter_group.counters[SC_TYPE_STAMP].value));
  return (tstamp <= now - options->lifetime);
}

typedef struct _StatsTimerState
{
  GTimeVal now;
  time_t oldest_counter;
  gint dropped_counters;
  EVTREC *stats_event;
  StatsOptions *options;
} StatsTimerState;

static gboolean
stats_prune_cluster(StatsCluster *sc, StatsTimerState *st)
{
  gboolean expired;

  expired = stats_cluster_is_expired(st->options, sc, st->now.tv_sec);
  if (expired)
    {
      time_t tstamp = atomic_gssize_racy_get(&(sc->counter_group.counters[SC_TYPE_STAMP].value));
      if ((st->oldest_counter) == 0 || st->oldest_counter > tstamp)
        st->oldest_counter = tstamp;
      st->dropped_counters++;
      stats_query_deindex_cluster(sc);
    }
  return expired;
}

static gboolean
stats_format_and_prune_cluster(StatsCluster *sc, gpointer user_data)
{
  StatsTimerState *st = (StatsTimerState *) user_data;

  if (st->stats_event)
    stats_log_format_cluster(sc, st->stats_event);
  return stats_prune_cluster(sc, st);
}

void
stats_publish_and_prune_counters(StatsOptions *options)
{
  StatsTimerState st;
  gboolean publish = (options->log_freq > 0);

  st.oldest_counter = 0;
  st.dropped_counters = 0;
  st.stats_event = NULL;
  st.options = options;
  cached_g_current_time(&st.now);

  if (publish)
    st.stats_event = msg_event_create(EVT_PRI_INFO, "Log statistics", NULL);

  stats_lock();
  stats_foreach_cluster_remove(stats_format_and_prune_cluster, &st);
  stats_unlock();

  if (publish)
    msg_event_send(st.stats_event);

  if (st.dropped_counters > 0)
    {
      msg_notice("Pruning stats-counters have finished",
                 evt_tag_int("dropped", st.dropped_counters),
                 evt_tag_long("oldest-timestamp", (long) st.oldest_counter));
    }
}


static void
stats_timer_rearm(StatsOptions *options, struct iv_timer *timer)
{
  glong freq = options->log_freq;

  if (!freq)
    freq = options->lifetime <= 1 ? 1 : options->lifetime / 2;
  if (freq > 0)
    {
      /* arm the timer */
      iv_validate_now();
      timer->expires = iv_now;
      timespec_add_msec(&timer->expires, freq * 1000);
      iv_timer_register(timer);
    }
}

static void
stats_timer_init(struct iv_timer *timer, void (*handler)(void *), StatsOptions *options)
{
  IV_TIMER_INIT(timer);
  timer->handler = handler;
  timer->cookie = options;
}

static void
stats_timer_kill(struct iv_timer *timer)
{
  if (!timer->handler)
    return;
  if (iv_timer_registered(timer))
    iv_timer_unregister(timer);
}

static struct iv_timer stats_timer;

static void
stats_timer_elapsed(gpointer st)
{
  StatsOptions *options = (StatsOptions *) st;

  stats_publish_and_prune_counters(options);
  stats_timer_rearm(options, &stats_timer);
}

void
stats_timer_reinit(StatsOptions *options)
{
  stats_timer_kill(&stats_timer);
  stats_timer_init(&stats_timer, stats_timer_elapsed, options);
  stats_timer_rearm(options, &stats_timer);
}

static StatsOptions *stats_options;

void
stats_reinit(StatsOptions *options)
{
  stats_options = options;
  stats_timer_reinit(options);
}

void
stats_init(void)
{
  stats_cluster_init();
  stats_registry_init();
  stats_aggregator_registry_init();
  stats_query_init();
}

void
stats_destroy(void)
{
  stats_query_deinit();
  stats_registry_deinit();
  stats_aggregator_registry_deinit();
  stats_cluster_deinit();
}

void
stats_options_defaults(StatsOptions *options)
{
  options->level = 0;
  options->log_freq = 600;
  options->lifetime = 600;
  options->max_dynamic = -1;
}

gboolean
stats_check_level(gint level)
{
  if (stats_options)
    return (stats_options->level >= level);
  else
    return level == 0;
}

gboolean
stats_check_dynamic_clusters_limit(guint number_of_clusters)
{
  if (!stats_options)
    return TRUE;
  if (stats_options->max_dynamic == -1)
    return TRUE;
  return (stats_options->max_dynamic > number_of_clusters);
}

gint
stats_number_of_dynamic_clusters_limit(void)
{
  if (!stats_options)
    return -1;
  return stats_options->max_dynamic;
}
