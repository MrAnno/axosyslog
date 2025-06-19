/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 1998-2013 Balázs Scheidler
 * Copyright (c) 2024 László Várady
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
#ifndef POLL_FILE_CHANGES_H_INCLUDED
#define POLL_FILE_CHANGES_H_INCLUDED

#include "poll-events.h"
#include "logpipe.h"

#include <iv.h>

typedef struct _PollFileChanges PollFileChanges;

struct _PollFileChanges
{
  PollEvents super;
  gint fd;
  gchar *follow_filename;
  gint follow_freq;
  struct iv_timer follow_timer;
  LogPipe *control;

  gboolean stop_on_eof;
  void (*on_read)(PollFileChanges *);
  gboolean (*on_eof)(PollFileChanges *);
  void (*on_file_moved)(PollFileChanges *);
};

PollEvents *poll_file_changes_new(gint fd, const gchar *follow_filename, gint follow_freq, LogPipe *control);

void poll_file_changes_init_instance(PollFileChanges *self, gint fd, const gchar *follow_filename, gint follow_freq,
                                     LogPipe *control);
void poll_file_changes_update_watches(PollEvents *s, GIOCondition cond);
void poll_file_changes_stop_watches(PollEvents *s);
void poll_file_changes_stop_on_eof(PollEvents *s);
void poll_file_changes_free(PollEvents *s);

#endif
