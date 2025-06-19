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
from axosyslog_light.common.blocking import wait_until_true
from axosyslog_light.helpers.loggen.loggen import LoggenStartParams

NUMBER_OF_MESSAGES = 10


def test_pp_with_simple_tcp_connection(config, port_allocator, syslog_ng, loggen):
    config.update_global_options(stats_level=1)
    network_source = config.create_network_source(ip="localhost", port=port_allocator(), transport="proxied-tcp")
    file_destination = config.create_file_destination(file_name="output.log")
    config.create_logpath(statements=[network_source, file_destination])

    syslog_ng.start(config)

    loggen.start(
        LoggenStartParams(
            target=network_source.options["ip"],
            port=network_source.options["port"],
            inet=True,
            stream=True,
            number=NUMBER_OF_MESSAGES,
        ),
    )
    wait_until_true(lambda: loggen.get_sent_message_count() == NUMBER_OF_MESSAGES)

    assert network_source.get_stats()["processed"] == 0
