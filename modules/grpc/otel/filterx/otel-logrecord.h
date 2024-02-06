/*
 * Copyright (c) 2023 shifter
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
  */

#ifndef OTEL_LOG_RECORD_H
#define OTEL_LOG_RECORD_H

#include "compat/cpp-start.h"
#include "filterx/filterx-object.h"
#include "plugin.h"

gpointer grpc_otel_filterx_logrecord_contruct_new(Plugin *self);
FilterXObject *otel_logrecord_new(GPtrArray *args);

#include "compat/cpp-end.h"

FILTERX_DECLARE_TYPE(olr);

#endif