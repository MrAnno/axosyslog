/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 1998-2011 Balázs Scheidler
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
 */

#ifndef TRANSPORT_TLS_VERIFIER_H_INCLUDED
#define TRANSPORT_TLS_VERIFIER_H_INCLUDED

#include "syslog-ng.h"
#include "atomic.h"
#include <openssl/ssl.h>

typedef gint (*TLSSessionVerifyFunc)(gint ok, X509_STORE_CTX *ctx, gpointer user_data);
typedef struct _TLSVerifier
{
  GAtomicCounter ref_cnt;
  TLSSessionVerifyFunc verify_func;
  gpointer verify_data;
  GDestroyNotify verify_data_destroy;
} TLSVerifier;

TLSVerifier *tls_verifier_new(TLSSessionVerifyFunc verify_func, gpointer verify_data,
                              GDestroyNotify verify_data_destroy);
TLSVerifier *tls_verifier_ref(TLSVerifier *self);
void tls_verifier_unref(TLSVerifier *self);

gboolean tls_verify_certificate_name(X509 *cert, const gchar *hostname);

gboolean tls_wildcard_match(const gchar *host_name, const gchar *pattern);


#endif
