/*
 * Copyright (c) 2019 Balabit
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

#include "syslog-ng.h"
#include "compat/amqp-compat.h"

#ifdef SYSLOG_NG_HAVE_RABBITMQ_C_TCP_SOCKET_H
#include <rabbitmq-c/ssl_socket.h>
#else
#include <amqp_ssl_socket.h>
#endif

void
amqp_compat_set_verify(amqp_socket_t *self, gboolean verify)
{
#ifdef SYSLOG_NG_HAVE_AMQP_SSL_SOCKET_SET_VERIFY_PEER
  amqp_ssl_socket_set_verify_peer(self, verify);
  amqp_ssl_socket_set_verify_hostname(self, verify);
#else
  amqp_ssl_socket_set_verify(self, verify);
#endif
}
