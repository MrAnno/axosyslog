/*
 * Copyright (c) 2024 Axoflow
 * Copyright (c) 2023-2024 Attila Szakacs <attila.szakacs@axoflow.com>
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

#include "cfg-parser.h"
#include "plugin.h"
#include "plugin-types.h"
#include "filterx/func-update-metric.h"

extern CfgParser metrics_probe_parser;

static Plugin metrics_probe_plugins[] =
{
  {
    .type = LL_CONTEXT_PARSER,
    .name = "metrics-probe",
    .parser = &metrics_probe_parser,
  },
  FILTERX_FUNCTION_PLUGIN(update_metric),
};

gboolean
metrics_probe_module_init(PluginContext *context, CfgArgs *args)
{
  plugin_register(context, metrics_probe_plugins, G_N_ELEMENTS(metrics_probe_plugins));
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "metrics-probe",
  .version = SYSLOG_NG_VERSION,
  .description = "The metrics-probe module provides a way to produce custom metrics for syslog-ng.",
  .core_revision = SYSLOG_NG_SOURCE_REVISION,
  .plugins = metrics_probe_plugins,
  .plugins_len = G_N_ELEMENTS(metrics_probe_plugins),
};
