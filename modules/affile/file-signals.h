/*
 * Copyright (c) 2022 Shikhar Vashistha
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

#ifndef FILE_SIGNALS_H_INCLUDED
#define FILE_SIGNALS_H_INCLUDED

#include "signal-slot-connector/signal-slot-connector.h"

typedef struct _FileFlushSignalData FileFlushSignalData;
typedef struct _FileReopener FileReopener;

struct _FileFlushSignalData
{
  gchar *filename;
  gchar *persist_name;
  gsize size;
  FileReopener *reopener;
  gchar *last_rotation_time;
};

#define signal_file_flush SIGNAL(file_rotation, rotation_request, FileFlushSignalData *)


struct _FileReopener
{
  void (*request_reopen)(gpointer cookie);
  gpointer cookie;
};

static inline void
file_reopener_request_reopen(FileReopener *self)
{
  self->request_reopen(self->cookie);
}

#endif