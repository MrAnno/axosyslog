/*
 * Copyright (c) 2002-2011 Balabit
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
 *
 */

#include "pacct-format.h"
#include "messages.h"
#include "plugin.h"
#include "plugin-types.h"

static gpointer
pacct_format_construct(Plugin *self)
{
  return &pacct_handler;
}

static Plugin pacct_format_plugin =
{
  .type = LL_CONTEXT_FORMAT,
  .name = "pacct",
  .construct = pacct_format_construct,
};

gboolean
pacctformat_module_init(PluginContext *context, CfgArgs *args)
{
  plugin_register(context, &pacct_format_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "pacctformat",
  .version = SYSLOG_NG_VERSION,
  .description = "The pacctformat module provides support for parsing BSD Process Accounting files",
  .core_revision = SYSLOG_NG_SOURCE_REVISION,
  .plugins = &pacct_format_plugin,
  .plugins_len = 1,
};
