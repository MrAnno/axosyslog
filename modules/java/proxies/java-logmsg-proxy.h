/*
 * Copyright (c) 2010-2015 Balabit
 * Copyright (c) 2010-2015 Viktor Juhasz <viktor.juhasz@balabit.com>
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


#ifndef JAVA_LOGMSG_PROXY_H_
#define JAVA_LOGMSG_PROXY_H_

#include "org_syslog_ng_LogMessage.h"
#include "logmsg/logmsg.h"

typedef struct _JavaLogMessageProxy JavaLogMessageProxy;

JavaLogMessageProxy *java_log_message_proxy_new(void);
void java_log_message_proxy_free(JavaLogMessageProxy *self);

jobject java_log_message_proxy_create_java_object(JavaLogMessageProxy *self, LogMessage *msg);

#endif /* JAVA_LOGMSG_PROXY_H_ */
