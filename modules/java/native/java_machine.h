/*
 * Copyright (c) 2014 Balabit
 * Copyright (c) 2014 Viktor Juhasz <viktor.juhasz@balabit.com>
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

#ifndef JAVA_MACHINE_H
#define JAVA_MACHINE_H 1

#include <jni.h>
#include <glib.h>
#include "java-class-loader.h"

#define CALL_JAVA_FUNCTION_VOID(env, function, ...) (*(env))->function(env)
#define CALL_JAVA_FUNCTION(env, function, ...) (*(env))->function(env, __VA_ARGS__)

typedef struct _JavaVMSingleton JavaVMSingleton;

JavaVMSingleton *java_machine_ref(void);
void java_machine_unref(JavaVMSingleton *self);
gboolean java_machine_start(JavaVMSingleton *self, const gchar *jvm_options);

void java_machine_detach_thread(void);

JNIEnv *java_machine_get_env(JavaVMSingleton *self);

jclass java_machine_load_class(JavaVMSingleton *self, const gchar *class_name, const gchar *class_path);

#endif
