/*
 * Copyright (c) 2024 Balazs Scheidler <balazs.scheidler@axoflow.com>
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

#ifndef TRANSPORT_ADAPTER_H_INCLUDED
#define TRANSPORT_ADAPTER_H_INCLUDED

#include "transport-stack.h"

typedef struct _LogTransportAdapter LogTransportAdapter;
struct _LogTransportAdapter
{
  LogTransport super;
  LogTransportIndex base_index;
};

gssize log_transport_adapter_read_method(LogTransport *s, gpointer buf, gsize buflen, LogTransportAuxData *aux);
gssize log_transport_adapter_write_method(LogTransport *s, const gpointer buf, gsize count);
gssize log_transport_adapter_writev_method(LogTransport *s, struct iovec *iov, gint iov_count);
void log_transport_adapter_shutdown_method(LogTransport *s);

void log_transport_adapter_init_instance(LogTransportAdapter *self, const gchar *name,
                                         LogTransportIndex base);

void log_transport_adapter_free_method(LogTransport *s);

#endif
