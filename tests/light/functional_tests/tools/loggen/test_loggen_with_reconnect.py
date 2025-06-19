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
from axosyslog_light.helpers.loggen.loggen import LoggenStartParams

NUMBER_OF_MESSAGES = 3


def test_loggen_with_reconnect(config, port_allocator, syslog_ng, loggen):
    network_source = config.create_network_source(ip="localhost", port=port_allocator())
    file_destination = config.create_file_destination(file_name="output.log")
    config.create_logpath(statements=[network_source, file_destination])

    syslog_ng.start(config)

    loggen.start(
        LoggenStartParams(
            target=network_source.options["ip"],
            port=network_source.options["port"],
            inet=True,
            rate=1,
            active_connections=1,
            reconnect=True,
            interval=30,
            number=NUMBER_OF_MESSAGES,
        ),
    )

    syslog_ng.stop()
    syslog_ng.start(config)

    logs = file_destination.read_logs(counter=NUMBER_OF_MESSAGES - 1)

    # the send function of socket only write data to the output buffer, if the remote host is lost
    # the message in the buffer will be discarded, result in len(logs)=NUMBER_OF_MESSAGES-1
    assert len(logs) == NUMBER_OF_MESSAGES - 1
