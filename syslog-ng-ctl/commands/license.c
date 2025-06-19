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

#include "license.h"

static gboolean license_json = FALSE;

GOptionEntry license_options[] =
{
  { "json", 'J', 0, G_OPTION_ARG_NONE, &license_json, "enable json output", NULL },
  { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL }
};

gint
slng_license(int argc, char *argv[], const gchar *mode, GOptionContext *ctx)
{
  gchar buff[256];

  if (license_json)
    g_snprintf(buff, 255, "LICENSE JSON\n");
  else
    g_snprintf(buff, 255, "LICENSE\n");

  return dispatch_command(buff);
}
