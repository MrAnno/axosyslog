/*
 * Copyright (c) 2019 One Identity
 * Copyright (c) 2019 László Várady
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "syslog-ng.h"
#include "logsource.h"
#include "logmsg/logmsg.h"
#include "logpipe.h"
#include "cfg.h"
#include "apphook.h"

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include <string.h>

#define TEST_SOURCE_GROUP "test_source_group"
#define TEST_STATS_ID "test_stats_id"
#define TEST_STATS_INSTANCE "test_stats_instance"

GlobalConfig *cfg;
LogSourceOptions source_options;

void
setup(void)
{
  cfg = cfg_new_snippet();
  log_source_options_defaults(&source_options);
  app_startup();
}

void
teardown(void)
{
  app_shutdown();
  log_source_options_destroy(&source_options);
  cfg_free(cfg);
}

typedef struct TestSource
{
  LogSource super;
  gsize wakeup_count;
} TestSource;

void
test_source_wakeup(LogSource *s)
{
  TestSource *source = (TestSource *) s;
  source->wakeup_count++;
}

LogSource *
test_source_init(LogSourceOptions *options)
{
  TestSource *source = g_new0(TestSource, 1);
  log_source_init_instance(&source->super, cfg);
  source->super.wakeup = test_source_wakeup;

  log_source_options_init(options, cfg, TEST_SOURCE_GROUP);
  log_source_set_options(&source->super, options, TEST_STATS_ID, TEST_STATS_INSTANCE, TRUE, FALSE, NULL);
  cr_assert(log_pipe_init(&source->super.super));
  return &source->super;
}

void
test_source_destroy(LogSource *source)
{
  log_pipe_deinit(&source->super);
  log_pipe_unref(&source->super);
}


const gchar *
resolve_sockaddr_to_hostname(gsize *result_len, GSockAddr *saddr, const HostResolveOptions *host_resolve_options)
{
  static const gchar *test_hostname = "resolved-test-host";
  *result_len = strlen(test_hostname);
  return test_hostname;
}

typedef struct _MangleHostnameParams
{
  gboolean keep_hostname;
  gboolean chain_hostnames;
  const gchar *input_host;
  const gchar *expected_hostname;

  guint32 msg_flags;
} MangleHostnameParams;

ParameterizedTestParameters(log_source, test_mangle_hostname)
{
  static MangleHostnameParams test_params[] =
  {
    { .keep_hostname = TRUE, .chain_hostnames = FALSE, "msg-test-host", "msg-test-host" },
    { .keep_hostname = TRUE, .chain_hostnames = FALSE, NULL, "resolved-test-host" },
    { .keep_hostname = TRUE, .chain_hostnames = FALSE, "", "resolved-test-host" },
    { .keep_hostname = TRUE, .chain_hostnames = TRUE, "msg-test-host", "msg-test-host" },
    { .keep_hostname = FALSE, .chain_hostnames = TRUE, NULL, "resolved-test-host/resolved-test-host" },
    { .keep_hostname = FALSE, .chain_hostnames = TRUE, "", "resolved-test-host/resolved-test-host" },
    { .keep_hostname = FALSE, .chain_hostnames = FALSE, "msg-test-host", "resolved-test-host" },
    { .keep_hostname = FALSE, .chain_hostnames = TRUE, "msg-test-host", "msg-test-host/resolved-test-host" },
    {
      .keep_hostname = FALSE, .chain_hostnames = TRUE, "msg-test-host", TEST_SOURCE_GROUP "@resolved-test-host",
      .msg_flags = LF_LOCAL
    },
    {
      .keep_hostname = FALSE, .chain_hostnames = TRUE, "msg-test-host", "resolved-test-host",
      .msg_flags = LF_LOCAL | LF_SIMPLE_HOSTNAME
    },
  };

  return cr_make_param_array(MangleHostnameParams, test_params, G_N_ELEMENTS(test_params));
}

ParameterizedTest(MangleHostnameParams *test_params, log_source, test_mangle_hostname)
{
  source_options.keep_hostname = test_params->keep_hostname;
  source_options.chain_hostnames = test_params->chain_hostnames;
  LogSource *source = test_source_init(&source_options);

  LogMessage *msg = log_msg_new_empty();

  if (test_params->input_host)
    log_msg_set_value(msg, LM_V_HOST, test_params->input_host, -1);

  msg->flags |= test_params->msg_flags;

  log_source_mangle_hostname(source, msg);

  const gchar *actual_hostname = log_msg_get_value(msg, LM_V_HOST, NULL);
  cr_assert_str_eq(actual_hostname, test_params->expected_hostname);

  log_msg_unref(msg);
  test_source_destroy(source);
}

Test(log_source, test_chain_hostname_truncates_long_chained_hostnames)
{
  source_options.chain_hostnames = TRUE;
  LogSource *source = test_source_init(&source_options);
  LogMessage *msg = log_msg_new_empty();

  const gsize long_hostname_size = 512;
  gchar long_hostname[long_hostname_size];
  memset(long_hostname, 'Z', long_hostname_size);
  log_msg_set_value(msg, LM_V_HOST, long_hostname, long_hostname_size);

  log_source_mangle_hostname(source, msg);

  const gchar *actual_hostname = log_msg_get_value(msg, LM_V_HOST, NULL);
  gsize expected_hostname_len = 255;

  cr_assert_eq(strlen(actual_hostname), expected_hostname_len);
  cr_assert_arr_eq(actual_hostname, long_hostname, expected_hostname_len);

  log_msg_unref(msg);
  test_source_destroy(source);
}

Test(log_source, test_host_and_program_override)
{
  source_options.host_override = g_strdup("test-host-override");
  source_options.program_override = g_strdup("test-program-override");
  LogSource *source = test_source_init(&source_options);

  LogMessage *msg = log_msg_new_empty();
  log_msg_set_value(msg, LM_V_HOST, "hostname-to-override", -1);
  log_msg_set_value(msg, LM_V_PROGRAM, "program-to-override", -1);

  log_msg_ref(msg);
  log_source_post(source, msg);

  const gchar *actual_hostname = log_msg_get_value(msg, LM_V_HOST, NULL);
  cr_expect_str_eq(actual_hostname, source_options.host_override);
  const gchar *actual_program = log_msg_get_value(msg, LM_V_PROGRAM, NULL);
  cr_expect_str_eq(actual_program, source_options.program_override);

  log_msg_unref(msg);
  test_source_destroy(source);
}

TestSuite(log_source, .init = setup, .fini = teardown);
