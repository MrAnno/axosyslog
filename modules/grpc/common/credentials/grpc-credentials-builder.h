/*
 * Copyright (c) 2023 Attila Szakacs
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

#ifndef GRPC_CREDENTIALS_BUILDER_H
#define GRPC_CREDENTIALS_BUILDER_H

#include "syslog-ng.h"

#include "compat/cpp-start.h"

/* Server */

typedef enum
{
  GSAM_INSECURE,
  GSAM_TLS,
  GSAM_ALTS,
} GrpcServerAuthMode;

typedef enum
{
  GSTPV_OPTIONAL_UNTRUSTED,
  GSTPV_OPTIONAL_TRUSTED,
  GSTPV_REQUIRED_UNTRUSTED,
  GSTPV_REQUIRED_TRUSTED,
} GrpcServerTlsPeerVerify;

typedef struct GrpcServerCredentialsBuilderW_ GrpcServerCredentialsBuilderW; // Wrapper struct

void grpc_server_credentials_builder_set_mode(GrpcServerCredentialsBuilderW *s, GrpcServerAuthMode mode);
gboolean grpc_server_credentials_builder_set_tls_ca_path(GrpcServerCredentialsBuilderW *s, const gchar *ca_path);
gboolean grpc_server_credentials_builder_set_tls_key_path(GrpcServerCredentialsBuilderW *s, const gchar *key_path);
gboolean grpc_server_credentials_builder_set_tls_cert_path(GrpcServerCredentialsBuilderW *s, const gchar *cert_path);
void grpc_server_credentials_builder_set_tls_peer_verify(GrpcServerCredentialsBuilderW *s,
                                                         GrpcServerTlsPeerVerify peer_verify);

/* Client */

typedef enum
{
  GCAM_INSECURE,
  GCAM_TLS,
  GCAM_ALTS,
  GCAM_ADC,
  GCAM_SERVICE_ACCOUNT,
} GrpcClientAuthMode;

typedef struct GrpcClientCredentialsBuilderW_ GrpcClientCredentialsBuilderW; // Wrapper struct

void grpc_client_credentials_builder_set_mode(GrpcClientCredentialsBuilderW *s, GrpcClientAuthMode mode);
gboolean grpc_client_credentials_builder_set_tls_ca_path(GrpcClientCredentialsBuilderW *s, const gchar *ca_path);
gboolean grpc_client_credentials_builder_set_tls_key_path(GrpcClientCredentialsBuilderW *s, const gchar *key_path);
gboolean grpc_client_credentials_builder_set_tls_cert_path(GrpcClientCredentialsBuilderW *s, const gchar *cert_path);
void grpc_client_credentials_builder_add_alts_target_service_account(GrpcClientCredentialsBuilderW *s,
    const gchar *target_service_account);
gboolean grpc_client_credentials_builder_service_account_set_key(GrpcClientCredentialsBuilderW *s,
    const gchar *key_path);
void grpc_client_credentials_builder_service_account_set_validity_duration(GrpcClientCredentialsBuilderW *s,
    guint64 validity_duration);

#include "compat/cpp-end.h"

#endif
