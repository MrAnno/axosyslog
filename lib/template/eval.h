/*
 * Copyright (c) 2020 Balabit
 * Copyright (c) 2020 Balazs Scheidler <bazsi77@gmail.com>
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

#ifndef TEMPLATE_EVAL_H_INCLUDED
#define TEMPLATE_EVAL_H_INCLUDED 1

#include "syslog-ng.h"
#include "common-template-typedefs.h"
#include "escaping.h"
#include "logmsg/logmsg.h"

typedef struct _LogTemplateEvalOptions
{
  /* options for recursive template evaluation, inherited from the parent */
  const LogTemplateOptions *opts;
  gint tz;
  gint seq_num;
  const gchar *context_id;
  LogMessageValueType context_id_type;
  LogTemplateEscapeFunction escape;
} LogTemplateEvalOptions;

#define DEFAULT_TEMPLATE_EVAL_OPTIONS ((LogTemplateEvalOptions){NULL, LTZ_LOCAL, 0, NULL, LM_VT_STRING})

void log_template_format(LogTemplate *self, LogMessage *lm, LogTemplateEvalOptions *options, GString *result);
void log_template_format_value_and_type(LogTemplate *self, LogMessage *lm, LogTemplateEvalOptions *options,
                                        GString *result, LogMessageValueType *type);
void log_template_append_format(LogTemplate *self, LogMessage *lm, LogTemplateEvalOptions *options, GString *result);
void log_template_append_format_value_and_type(LogTemplate *self, LogMessage *lm, LogTemplateEvalOptions *options,
                                               GString *result, LogMessageValueType *type);
void log_template_append_format_with_context(LogTemplate *self, LogMessage **messages, gint num_messages,
                                             LogTemplateEvalOptions *options, GString *result);
void log_template_append_format_value_and_type_with_context(LogTemplate *self, LogMessage **messages, gint num_messages,
                                                            LogTemplateEvalOptions *options,
                                                            GString *result, LogMessageValueType *type);
void log_template_format_value_and_type_with_context(LogTemplate *self, LogMessage **messages, gint num_messages,
                                                     LogTemplateEvalOptions *options,
                                                     GString *result, LogMessageValueType *type);
void log_template_format_with_context(LogTemplate *self, LogMessage **messages, gint num_messages,
                                      LogTemplateEvalOptions *options, GString *result);

guint log_template_hash(LogTemplate *self, LogMessage *lm, LogTemplateEvalOptions *options);

#endif
