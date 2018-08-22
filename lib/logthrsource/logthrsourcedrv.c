/*
 * Copyright (c) 2018 Balabit
 * Copyright (c) 2018 László Várady <laszlo.varady@balabit.com>
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

/* TODO: scratch-buffers GC call + reclaim  */

#include "logthrsourcedrv.h"
#include "mainloop-worker.h"

#include <iv.h>
#include <iv_event.h>

typedef struct _WakeupCondition
{
  GMutex *lock;
  GCond *cond;
  gboolean awoken;
} WakeupCondition;

struct _LogThreadedSourceWorker
{
  LogSource super;
  LogThreadedSourceDriver *control;
  WakeupCondition wakeup_cond;
  WorkerOptions options;

  LogThreadedSourceWorkerRun run;
  LogThreadedSourceWorkerRequestExit request_exit;
  LogThreadedSourceWorkerWakeup wakeup;
};

static LogPipe *
log_threaded_source_worker_logpipe(LogThreadedSourceWorker *self)
{
  return &self->super.super;
}

static void
log_threaded_source_worker_set_options(LogThreadedSourceWorker *self, LogThreadedSourceDriver *control,
                                       LogThreadedSourceWorkerOptions *options,
                                       const gchar *stats_id, const gchar *stats_instance)
{
  /* TODO: support position tracking */
  log_source_set_options(&self->super, &options->super, stats_id, stats_instance, TRUE, FALSE,
                         control->super.super.super.expr_node);

  log_pipe_unref(&self->control->super.super.super);
  log_pipe_ref(&control->super.super.super);
  self->control = control;
}

void
log_threaded_source_worker_options_defaults(LogThreadedSourceWorkerOptions *options)
{
  log_source_options_defaults(&options->super);
}

void
log_threaded_source_worker_options_init(LogThreadedSourceWorkerOptions *options, GlobalConfig *cfg,
                                        const gchar *group_name)
{
  log_source_options_init(&options->super, cfg, group_name);
}

void
log_threaded_source_worker_options_destroy(LogThreadedSourceWorkerOptions *options)
{
  log_source_options_destroy(&options->super);
}

/* The wakeup lock must be held before calling this function. */
static void
log_threaded_source_worker_suspend(LogThreadedSourceWorker *self)
{
  self->wakeup_cond.awoken = FALSE;
  while (!self->wakeup_cond.awoken)
    g_cond_wait(self->wakeup_cond.cond, self->wakeup_cond.lock);
}

static void
log_threaded_source_worker_run(LogThreadedSourceWorker *self)
{
  msg_debug("Worker thread started", evt_tag_str("driver", self->control->super.super.id));

  self->run(self->control);

  msg_debug("Worker thread finished", evt_tag_str("driver", self->control->super.super.id));
}

static void
log_threaded_source_worker_request_exit(LogThreadedSourceWorker *self)
{
  /* TODO force exit after timeout */
  msg_debug("Requesting worker thread exit", evt_tag_str("driver", self->control->super.super.id));
  self->request_exit(self->control);
}

static void
_worker_wakeup(LogSource *s)
{
  LogThreadedSourceWorker *self = (LogThreadedSourceWorker *) s;

  self->wakeup(self->control);
}

static gboolean
log_threaded_source_worker_init(LogPipe *s)
{
  LogThreadedSourceWorker *self = (LogThreadedSourceWorker *) s;
  if (!log_source_init(s))
    return FALSE;

  g_assert(self->run);
  g_assert(self->request_exit);

  main_loop_create_worker_thread((WorkerThreadFunc) log_threaded_source_worker_run,
                                 (WorkerExitNotificationFunc) log_threaded_source_worker_request_exit,
                                 self, &self->options);

  return TRUE;
}

static void
log_threaded_source_worker_free(LogPipe *s)
{
  LogThreadedSourceWorker *self = (LogThreadedSourceWorker *) s;

  g_cond_free(self->wakeup_cond.cond);
  g_mutex_free(self->wakeup_cond.lock);

  log_source_free(s);
}

static LogThreadedSourceWorker *
log_threaded_source_worker_new(GlobalConfig *cfg)
{
  LogThreadedSourceWorker *self = g_new0(LogThreadedSourceWorker, 1);
  log_source_init_instance(&self->super, cfg);

  self->wakeup_cond.lock = g_mutex_new();
  self->wakeup_cond.cond = g_cond_new();

  self->options.is_external_input = TRUE;

  self->super.super.init = log_threaded_source_worker_init;
  /* TODO: deinit vs WorkerExitNotification order */
  self->super.super.free_fn = log_threaded_source_worker_free;
  self->super.wakeup = _worker_wakeup;

  return self;
}


gboolean
log_threaded_source_driver_init_method(LogPipe *s)
{
  LogThreadedSourceDriver *self = (LogThreadedSourceDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  if (!log_src_driver_init_method(s))
    return FALSE;

  g_assert(self->format_stats_instance);

  log_threaded_source_worker_options_init(&self->worker_options, cfg, self->super.super.group);
  log_threaded_source_worker_set_options(self->worker, self, &self->worker_options,
                                         self->super.super.id, self->format_stats_instance(self));
  // state

  LogPipe *worker_pipe = log_threaded_source_worker_logpipe(self->worker);
  log_pipe_append(worker_pipe, s);
  if (!log_pipe_init(worker_pipe))
    {
      log_pipe_unref(worker_pipe);
      self->worker = NULL;
      return FALSE;
    }

  return TRUE;
}

gboolean
log_threaded_source_driver_deinit_method(LogPipe *s)
{
  LogThreadedSourceDriver *self = (LogThreadedSourceDriver *) s;
  LogPipe *worker_pipe = log_threaded_source_worker_logpipe(self->worker);

  log_pipe_deinit(worker_pipe);

  return log_src_driver_deinit_method(s);
}

void
log_threaded_source_driver_free_method(LogPipe *s)
{
  LogThreadedSourceDriver *self = (LogThreadedSourceDriver *) s;
  LogPipe *worker_pipe = log_threaded_source_worker_logpipe(self->worker);

  log_pipe_unref(worker_pipe);
  self->worker = NULL;

  log_threaded_source_worker_options_destroy(&self->worker_options);
  log_src_driver_free(s);
}

void
log_threaded_source_driver_set_worker_run(LogThreadedSourceDriver *self, LogThreadedSourceWorkerRun run)
{
  self->worker->run = run;
}

void
log_threaded_source_driver_set_worker_request_exit(LogThreadedSourceDriver *self,
                                                   LogThreadedSourceWorkerRequestExit request_exit)
{
  self->worker->request_exit = request_exit;
}

static void
log_threaded_source_wakeup(LogThreadedSourceDriver *self)
{
  LogThreadedSourceWorker *worker = self->worker;

  g_mutex_lock(worker->wakeup_cond.lock);
  worker->wakeup_cond.awoken = TRUE;
  g_cond_signal(worker->wakeup_cond.cond);
  g_mutex_unlock(worker->wakeup_cond.lock);
}

void
log_threaded_source_set_wakeup(LogThreadedSourceDriver *self, LogThreadedSourceWorkerWakeup wakeup)
{
  self->worker->wakeup = wakeup;
}

void
log_threaded_source_post(LogThreadedSourceDriver *self, LogMessage *msg)
{
  /* TODO: offload (main_loop_io_worker_job_submit) */
  log_source_post(&self->worker->super, msg);
}

gboolean
log_threaded_source_free_to_send(LogThreadedSourceDriver *self)
{
  return log_source_free_to_send(&self->worker->super);
}

void
log_threaded_source_blocking_post(LogThreadedSourceDriver *self, LogMessage *msg)
{
  LogThreadedSourceWorker *worker = self->worker;

  log_threaded_source_post(self, msg);

  /*
   * The wakeup lock must be held before calling free_to_send() and suspend(),
   * otherwise g_cond_signal() might be called between free_to_send() and
   * suspend(). We'd hang in that case.
   *
   * LogReader does not have such a lock, but this is because it runs an ivykis
   * loop with a _synchronized_ event queue, where suspend() and the
   * "schedule_wakeup" event are guaranteed to be scheduled in the right order.
   */

  g_mutex_lock(worker->wakeup_cond.lock);
  if (!log_threaded_source_free_to_send(self))
    log_threaded_source_worker_suspend(worker);
  g_mutex_unlock(worker->wakeup_cond.lock);
}

void
log_threaded_source_driver_init_instance(LogThreadedSourceDriver *self, GlobalConfig *cfg)
{
  log_src_driver_init_instance(&self->super, cfg);

  log_threaded_source_worker_options_defaults(&self->worker_options);

  self->worker = log_threaded_source_worker_new(cfg);
  self->worker->wakeup = log_threaded_source_wakeup;

  self->super.super.super.init = log_threaded_source_driver_init_method;
  self->super.super.super.deinit = log_threaded_source_driver_deinit_method;
  self->super.super.super.free_fn = log_threaded_source_driver_free_method;
}
