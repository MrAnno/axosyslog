#!/usr/bin/env python
#############################################################################
# Copyright (c) 2024 Axoflow
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# As an additional exemption you are allowed to compile & link against the
# OpenSSL libraries as published by the OpenSSL project. See the file
# COPYING for details.
#
#############################################################################
import pytest

from src.common.file import copy_shared_file


def test_tls_verifier_reload_crash(config, syslog_ng, port_allocator, testcase_parameters):
    server_cert_path = copy_shared_file(testcase_parameters, "valid-localhost.crt")

    network_source = config.create_network_source(port=port_allocator())
    network_destination = config.create_network_destination(
        ip="localhost", port=port_allocator(), transport="tls",
        keep_alive="yes",
        tls={
                "ca-file": server_cert_path,
                "peer-verify": "yes",
            }
    )

    config.create_logpath(statements=[network_source, network_destination])

    network_destination.start_listener()
    syslog_ng.start(config)

    syslog_ng.reload(config)
    network_source.write_log("test msg")

    assert network_destination.read_until_logs(["test msg"])
