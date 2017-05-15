/*
 * Copyright (c) 2017 Balabit
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
 */

#include <criterion/criterion.h>

#include "snmptrapd-parser.h"
#include "testutils.h"
#include "apphook.h"
#include "msg_parse_lib.h"

#define SIZE_OF_ARRAY(array) (sizeof(array) / sizeof((array)[0]))

typedef struct
{
  const gchar *name;
  const gchar *value;
} TestNameValue;

static LogParser *
create_parser(void)
{
  LogParser *snmptrapd_parser = snmptrapd_parser_new(configuration);
  log_pipe_init((LogPipe *)snmptrapd_parser);
  return snmptrapd_parser;
}

static void
destroy_parser(LogParser *snmptrapd_parser)
{
  log_pipe_deinit((LogPipe *)snmptrapd_parser);
  log_pipe_unref((LogPipe *)snmptrapd_parser);
}

static LogMessage *
parse_str_into_log_message(LogParser *parser, const gchar *message)
{
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
  LogMessage *msg = log_msg_new_empty();

  log_msg_set_value(msg, LM_V_MESSAGE, message, -1);

  cr_assert(log_parser_process_message(parser, &msg, &path_options));

  return msg;
}

static void
assert_log_message_name_values(const gchar *input, TestNameValue *expected, gsize number_of_expected)
{
  LogParser *parser = create_parser();
  LogMessage *msg = parse_str_into_log_message(parser, input);

  for (int i=0; i < number_of_expected; i++)
    assert_log_message_value_by_name(msg, expected[i].name, expected[i].value);

  log_msg_unref(msg);
  destroy_parser(parser);
}

void
setup(void)
{
  configuration = cfg_new(0x0390);
  app_startup();
}

void
teardown(void)
{
  app_shutdown();
  cfg_free(configuration);
}

TestSuite(snmptrapd_parser, .init = setup, .fini = teardown);

Test(snmptrapd_parser, test_general_v2_message_with_oids)
{
  const gchar *input =
    "2017-05-10 12:46:14 web2-kukorica.syslog_ng.balabit [UDP: [127.0.0.1]:34257->[127.0.0.1]:162]:\n"
    "iso.3.6.1.2.1.1.3.0 = Timeticks: (875496867) 101 days, 7:56:08.67\t"
    "iso.3.6.1.6.3.1.1.4.1.0 = OID: iso.3.6.1.4.1.8072.2.3.0.1       "
    "iso.3.6.1.4.1.8072.2.3.2.1 = INTEGER: 60        \t "
    "iso.3.6.1.4.1.8072.2.1.3 = \"\"";

  TestNameValue expected[] =
  {
    { "HOST", "web2-kukorica.syslog_ng.balabit" },
    { ".snmp.transport_info", "UDP: [127.0.0.1]:34257->[127.0.0.1]:162" },
    { ".snmp.iso.3.6.1.2.1.1.3.0", "(875496867) 101 days, 7:56:08.67" },
    { ".snmp.iso.3.6.1.6.3.1.1.4.1.0", "iso.3.6.1.4.1.8072.2.3.0.1" },
    { ".snmp.iso.3.6.1.4.1.8072.2.3.2.1", "60" },
    { ".snmp.iso.3.6.1.4.1.8072.2.1.3", "" },
  };

  assert_log_message_name_values(input, expected, SIZE_OF_ARRAY(expected));
}

Test(snmptrapd_parser, test_general_v1_message_with_oids)
{
  const gchar *input =
    "2017-05-10 13:23:16 localhost [UDP: [127.0.0.1]:53831->[127.0.0.1]:162]: iso.3.6.1.4.1.8072.2.3.1\n"
    "\t Enterprise Specific Trap (.17) Uptime: 18:41:07.83\n"
    "iso.3.6.1.4.1.8072.2.1.1 = INTEGER: 123456";

  TestNameValue expected[] =
  {
    { "HOST", "localhost" },
    { ".snmp.transport_info", "UDP: [127.0.0.1]:53831->[127.0.0.1]:162" },
    { ".snmp.enterprise_oid", "iso.3.6.1.4.1.8072.2.3.1" },
    { ".snmp.type", "Enterprise Specific Trap" },
    { ".snmp.subtype", ".17" },
    { ".snmp.uptime", "18:41:07.83" },
    { ".snmp.iso.3.6.1.4.1.8072.2.1.1", "123456" }
  };

  assert_log_message_name_values(input, expected, SIZE_OF_ARRAY(expected));
}

Test(snmptrapd_parser, test_v2_with_symbolic_names_and_various_types)
{
  const gchar *input =
    "2017-05-13 12:17:32 localhost [UDP: [127.0.0.1]:52407->[127.0.0.1]:162]:  \n "
    "mib-2.1.3.0 = Timeticks: (875496867) 101 days, 7:56:08.67 \t"
    "snmpModules.1.1.4.1.0 = OID: netSnmpExampleHeartbeatNotification "
    "netSnmpExampleHeartbeatRate = INTEGER: 60\t"
    "netSnmpExampleString = STRING: \"string innerkey='innervalue'\"\t"
    "org.2.2 = Gauge32: 22\t"
    "org.1.1 = Counter32: 11123123   "
    "org.5.3 = Hex-STRING: A0 BB CC DD EF \t"
    "org.8.8 = NULL\t"
    "dod.7 = IpAddress: 192.168.1.0\t  "
    "org.5.9 = STRING: \"@\"";

  TestNameValue expected[] =
  {
    { "HOST", "localhost" },
    { ".snmp.transport_info", "UDP: [127.0.0.1]:52407->[127.0.0.1]:162" },
    { ".snmp.snmpModules.1.1.4.1.0", "netSnmpExampleHeartbeatNotification" },
    { ".snmp.netSnmpExampleHeartbeatRate", "60" },
    { ".snmp.netSnmpExampleString", "string innerkey='innervalue'" },
    { ".snmp.org.2.2", "22" },
    { ".snmp.org.1.1", "11123123" },
    { ".snmp.org.5.3", "A0 BB CC DD EF" },
    { ".snmp.org.8.8", "NULL" },
    { ".snmp.dod.7", "192.168.1.0" },
    { ".snmp.org.5.9", "@" }
  };

  assert_log_message_name_values(input, expected, SIZE_OF_ARRAY(expected));
}

Test(snmptrapd_parser, test_v1_with_symbolic_names)
{
  const gchar *input =
    "2017-05-13 12:18:30  localhost  [UDP: [127.0.0.1]:58143->[127.0.0.1]:162] : netSnmpExampleNotification \n"
    "\t Warm Start Trap (1) Uptime:  27 days, 2:39:02.34\n "
    "netSnmpExampleInteger = INTEGER: 123456 \t netSnmpExampleString = STRING: random string";

  TestNameValue expected[] =
  {
    { "HOST", "localhost" },
    { ".snmp.transport_info", "UDP: [127.0.0.1]:58143->[127.0.0.1]:162" },
    { ".snmp.enterprise_oid", "netSnmpExampleNotification" },
    { ".snmp.type", "Warm Start Trap" },
    { ".snmp.subtype", "1" },
    { ".snmp.uptime", "27 days, 2:39:02.34" },
    { ".snmp.netSnmpExampleInteger", "123456" },
    { ".snmp.netSnmpExampleString", "random string" }
  };

  assert_log_message_name_values(input, expected, SIZE_OF_ARRAY(expected));
}
