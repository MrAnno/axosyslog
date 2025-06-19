/*
 * Copyright (c) 2018 Balabit
 * Copyright (c) 2024 Axoflow
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

#include "wildcard-file-reader.h"
#include "mainloop.h"
#include "poll-file-changes.h"

static inline const gchar *
_format_persist_name(const LogPipe *s)
{
  const FileReader *self = (const FileReader *)s;
  static gchar persist_name[1024];

  if (self->owner->super.super.persist_name)
    {
      g_snprintf(persist_name, sizeof(persist_name), "wildcard_file_sd.%s.curpos(%s)",
                 self->owner->super.super.persist_name, self->filename->str);
    }
  else
    g_snprintf(persist_name, sizeof(persist_name), "wildcard_file_sd_curpos(%s)", self->filename->str);

  return persist_name;
}

static inline const gchar *
_format_legacy_persist_name(const LogPipe *s)
{
  const FileReader *self = (const FileReader *)s;
  static gchar persist_name[1024];

  if (self->owner->super.super.persist_name)
    g_snprintf(persist_name, sizeof(persist_name), "affile_sd.%s.curpos", self->owner->super.super.persist_name);
  else
    g_snprintf(persist_name, sizeof(persist_name), "affile_sd_curpos(%s)", self->filename->str);

  return persist_name;
}

static gboolean
_update_legacy_persist_name(WildcardFileReader *self)
{
  GlobalConfig *cfg = log_pipe_get_config(&self->super.super);

  if (!cfg->state)
    return TRUE;

  const gchar *current_persist_name = _format_persist_name(&self->super.super);
  const gchar *legacy_persist_name = _format_legacy_persist_name(&self->super.super);

  if (persist_state_entry_exists(cfg->state, current_persist_name))
    return TRUE;

  if (!persist_state_entry_exists(cfg->state, legacy_persist_name))
    return TRUE;

  return persist_state_copy_entry(cfg->state, legacy_persist_name, current_persist_name);
}

static gboolean
_init(LogPipe *s)
{
  WildcardFileReader *self = (WildcardFileReader *)s;
  self->file_state.deleted = FALSE;
  self->file_state.last_eof = FALSE;

  _update_legacy_persist_name(self);

  return file_reader_init_method(s);
}

static gboolean
_deinit(LogPipe *s)
{
  WildcardFileReader *self = (WildcardFileReader *)s;
  if (iv_task_registered(&self->file_state_event_handler))
    {
      iv_task_unregister(&self->file_state_event_handler);
    }
  return file_reader_deinit_method(s);
}

static void
_deleted_file_eof(FileStateEvent *self, FileReader *reader)
{
  if (self && self->deleted_file_eof)
    {
      self->deleted_file_eof(reader, self->deleted_file_eof_user_data);
    }
}

static void
_schedule_state_change_handling(WildcardFileReader *self)
{
  main_loop_assert_main_thread();
  if (!iv_task_registered(&self->file_state_event_handler))
    {
      iv_task_register(&self->file_state_event_handler);
    }
}

static void
_on_eof(WildcardFileReader *self)
{
  if (self->file_state.deleted)
    {
      self->file_state.last_eof = TRUE;
      _schedule_state_change_handling(self);
    }
}

static void
_on_deleted(WildcardFileReader *self)
{
  /* File can be deleted only once,
   * so there is no need for checking the state
   * before we set it
   */
  self->file_state.deleted = TRUE;

  if (!self->super.reader || !self->super.reader->poll_events)
    {
      self->file_state.last_eof = TRUE;
      _schedule_state_change_handling(self);
      return;
    }

  poll_file_changes_stop_on_eof(self->super.reader->poll_events);
}

static void
_notify(LogPipe *s, gint notify_code, gpointer user_data)
{
  WildcardFileReader *self = (WildcardFileReader *)s;
  switch(notify_code)
    {
    case NC_FILE_DELETED:
      _on_deleted(self);
      break;
    case NC_FILE_EOF:
      _on_eof(self);
      break;
    default:
      file_reader_notify_method(s, notify_code, user_data);
      break;
    }
}

static void
_handle_file_state_event(gpointer s)
{
  WildcardFileReader *self = (WildcardFileReader *)s;
  msg_debug("File status changed",
            evt_tag_int("EOF", self->file_state.last_eof),
            evt_tag_int("DELETED", self->file_state.deleted),
            evt_tag_str("Filename", self->super.filename->str));
  if (self->file_state.deleted && self->file_state.last_eof)
    _deleted_file_eof(&self->file_state_event, &self->super);
}

void
wildcard_file_reader_on_deleted_file_eof(WildcardFileReader *self,
                                         FileStateEventCallback cb,
                                         gpointer user_data)
{
  self->file_state_event.deleted_file_eof = cb;
  self->file_state_event.deleted_file_eof_user_data = user_data;
}

gboolean
wildcard_file_reader_is_deleted(WildcardFileReader *self)
{
  return self->file_state.deleted;
}

WildcardFileReader *
wildcard_file_reader_new(const gchar *filename, FileReaderOptions *options, FileOpener *opener, LogSrcDriver *owner,
                         GlobalConfig *cfg)
{
  WildcardFileReader *self = g_new0(WildcardFileReader, 1);
  file_reader_init_instance(&self->super, filename, options, opener, owner, cfg);
  self->super.super.init = _init;
  self->super.super.notify = _notify;
  self->super.super.deinit = _deinit;
  self->super.super.generate_persist_name = _format_persist_name;
  IV_TASK_INIT(&self->file_state_event_handler);
  self->file_state_event_handler.cookie = self;
  self->file_state_event_handler.handler = _handle_file_state_event;
  return self;
}
