/*
 * Copyright (c) 2017 Balabit
 * Copyright (c) 1998-2017 Balázs Scheidler
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

#ifndef CONTROL_COMMANDS_H_INCLUDED
#define CONTROL_COMMANDS_H_INCLUDED

#include "control/control.h"
#include "mainloop.h"


ControlCommand *control_find_command(const char *cmd);
void control_register_command(const gchar *command_name,
                              ControlCommandFunc function, gpointer user_data,
                              gboolean threaded);
void control_replace_command(const gchar *command_name,
                             ControlCommandFunc function, gpointer user_data,
                             gboolean threaded);
void reset_control_command_list(void);

#endif
