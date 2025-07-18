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
#ifndef MAINLOOP_IO_WORKER_H_INCLUDED
#define MAINLOOP_IO_WORKER_H_INCLUDED 1

#include "mainloop-worker.h"

#include <iv_work.h>

typedef struct _MainLoopIOWorkerJob
{
  void (*engage)(gpointer user_data);
  void (*work)(gpointer user_data, gpointer arg);
  void (*completion)(gpointer user_data, gpointer arg);
  void (*release)(gpointer user_data);
  gpointer user_data;
  gpointer arg;
  gboolean working;
  struct iv_work_item work_item;
} MainLoopIOWorkerJob;

void main_loop_io_worker_job_init(MainLoopIOWorkerJob *self);
gboolean main_loop_io_worker_job_submit(MainLoopIOWorkerJob *self, gpointer arg);

#if SYSLOG_NG_HAVE_IV_WORK_POOL_SUBMIT_CONTINUATION
void main_loop_io_worker_job_submit_continuation(MainLoopIOWorkerJob *self, gpointer arg);
#endif

void main_loop_io_worker_add_options(GOptionContext *ctx);

void main_loop_io_worker_init(void);
void main_loop_io_worker_deinit(void);

#endif
