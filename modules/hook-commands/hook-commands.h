/*
 * Copyright (c) 2002-2018 Balabit
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

#ifndef HOOK_COMMANDS_H_INLCUDED
#define HOOK_COMMANDS_H_INLCUDED 1

#include "driver.h"

typedef struct _HookCommandsPlugin HookCommandsPlugin;

void hook_commands_plugin_set_startup(HookCommandsPlugin *s, const gchar *command);
void hook_commands_plugin_set_setup(HookCommandsPlugin *s, const gchar *command);
void hook_commands_plugin_set_teardown(HookCommandsPlugin *s, const gchar *command);
void hook_commands_plugin_set_shutdown(HookCommandsPlugin *s, const gchar *command);
HookCommandsPlugin *hook_commands_plugin_new(void);

#endif
