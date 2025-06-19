#!/usr/bin/env python
#############################################################################
# Copyright (c) 2020 One Identity
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# As an additional exemption you are allowed to compile & link against the
# OpenSSL libraries as published by the OpenSSL project. See the file
# COPYING for details.
#
#############################################################################

TEMPLATE = r'"${SOURCEIP} ${SOURCEPORT} ${DESTIP} ${DESTPORT} ${IP_PROTO} ${MESSAGE}\n"'
PROXY_VERSION = 1
PROXY_SRC_IP = "1.1.1.1"
PROXY_DST_IP = "2.2.2.2"
PROXY_SRC_PORT = 3333
PROXY_DST_PORT = 4444
INPUT_MESSAGES = ["message 0", "message 1", "message 2"]
EXPECTED_MESSAGE0 = "1.1.1.1 3333 2.2.2.2 4444 4 message 0"
EXPECTED_MESSAGE1 = "1.1.1.1 3333 2.2.2.2 4444 4 message 1"
EXPECTED_MESSAGE2 = "1.1.1.1 3333 2.2.2.2 4444 4 message 2"


def test_pp_reload(config, syslog_ng, port_allocator):
    network_source = config.create_network_source(ip="localhost", port=port_allocator(), transport='"proxied-tcp"', flags="no-parse")
    file_destination = config.create_file_destination(file_name="output.log", template=TEMPLATE)
    config.create_logpath(statements=[network_source, file_destination])

    syslog_ng.start(config)

    network_source.write_logs_with_proxy_header(PROXY_VERSION, PROXY_SRC_IP, PROXY_DST_IP, PROXY_SRC_PORT, PROXY_DST_PORT, INPUT_MESSAGES, rate=1)

    # With the current loggen implementation there is no way to properly timing messages.
    # Here I made an assumption that with rate=1, there will be messages which will arrive
    # into syslog-ng AFTER the reload. Timing the reload after the first message arrives.

    # Note: The worst case scenario in case of extreme slowness in the test environment, is
    # that syslog-ng will receive all the messages before reload. In this case the test will
    # not fill it's purpose, and do not test if the headers are reserved between reloads.
    # But at least the test wont be flaky, it will pass in this corner case too.

    assert file_destination.read_log() == EXPECTED_MESSAGE0

    syslog_ng.reload(config)

    assert file_destination.read_log() == EXPECTED_MESSAGE1
    assert file_destination.read_log() == EXPECTED_MESSAGE2
