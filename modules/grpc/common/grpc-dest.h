/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2023-2024 Attila Szakacs <attila.szakacs@axoflow.com>
 * Copyright (c) 2023 László Várady
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

#ifndef GRPC_DEST_H
#define GRPC_DEST_H

#include "syslog-ng.h"

#include "compat/cpp-start.h"

#include "driver.h"
#include "credentials/grpc-credentials-builder.h"

typedef enum
{
  GDRA_UNSET = 0,
  GDRA_DISCONNECT,
  GDRA_DROP,
  GDRA_RETRY,
  GDRA_SUCCESS,
} GrpcDestResponseAction;

typedef enum
{
  GDR_OK,
  GDR_UNAVAILABLE,
  GDR_CANCELLED,
  GDR_DEADLINE_EXCEEDED,
  GDR_ABORTED,
  GDR_OUT_OF_RANGE,
  GDR_DATA_LOSS,
  GDR_UNKNOWN,
  GDR_INVALID_ARGUMENT,
  GDR_NOT_FOUND,
  GDR_ALREADY_EXISTS,
  GDR_PERMISSION_DENIED,
  GDR_UNAUTHENTICATED,
  GDR_FAILED_PRECONDITION,
  GDR_UNIMPLEMENTED,
  GDR_INTERNAL,
  GDR_RESOURCE_EXHAUSTED,
} GrpcDestResponse;

typedef struct GrpcDestDriver_ GrpcDestDriver;

void grpc_dd_set_url(LogDriver *s, const gchar *url);
void grpc_dd_set_compression(LogDriver *s, gboolean enable);
void grpc_dd_set_batch_bytes(LogDriver *s, glong b);
void grpc_dd_set_keepalive_time(LogDriver *s, gint t);
void grpc_dd_set_keepalive_timeout(LogDriver *s, gint t);
void grpc_dd_set_keepalive_max_pings(LogDriver *s, gint p);
void grpc_dd_add_int_channel_arg(LogDriver *s, const gchar *name, glong value);
void grpc_dd_add_string_channel_arg(LogDriver *s, const gchar *name, const gchar *value);
gboolean grpc_dd_add_header(LogDriver *s, const gchar *name, LogTemplate *value);
gboolean grpc_dd_add_schema_field(LogDriver *d, const gchar *name, const gchar *type, LogTemplate *value);
void grpc_dd_set_protobuf_schema(LogDriver *d, const gchar *proto_path, GList *values);
void grpc_dd_set_response_action(LogDriver *d, GrpcDestResponse response, GrpcDestResponseAction action);

LogTemplateOptions *grpc_dd_get_template_options(LogDriver *d);
GrpcClientCredentialsBuilderW *grpc_dd_get_credentials_builder(LogDriver *s);

#include "compat/cpp-end.h"

#endif
