/*
 * Copyright (c) 2018 Balabit
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

#ifndef NONDUMPABLE_ALLOCATOR_H_INCLUDED
#define NONDUMPABLE_ALLOCATOR_H_INCLUDED

#include "compat/glib.h"

#define PUBLIC __attribute__ ((visibility ("default")))
#define INTERNAL __attribute__ ((visibility ("hidden")))

typedef void(*NonDumpableLogger)(gchar *summary, gchar *reason);

gpointer nondumpable_buffer_alloc(gsize len) PUBLIC;
void nondumpable_buffer_free(gpointer buffer) PUBLIC;
gpointer nondumpable_buffer_realloc(gpointer buffer, gsize len) PUBLIC;
gpointer nondumpable_memcpy(gpointer dest, gpointer src, gsize len) PUBLIC;
void nondumpable_mem_zero(gpointer s, gsize len) PUBLIC;
void nondumpable_setlogger(NonDumpableLogger _debug, NonDumpableLogger _fatal) PUBLIC;

#endif
