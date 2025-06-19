#!/usr/bin/env python
#############################################################################
# Copyright (c) 2023 Balazs Scheidler <balazs.scheidler@axoflow.com>
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
import pytest


test_parameters = [
    ("$(format-json test.*)\n", r'{"test":{"key2":"value2","key1":"value1"}}'),
    # transformations
    ("$(format-json test.* --add-prefix foo.)\n", r'{"foo":{"test":{"key2":"value2","key1":"value1"}}}'),
    ("$(format-json test.* --replace-prefix test=foobar)\n", r'{"foobar":{"key2":"value2","key1":"value1"}}'),
    ("$(format-json test.* --shift-levels 1)\n", r'{"key2":"value2","key1":"value1"}'),
    ("$(format-json test.* --shift 2)\n", r'{"st":{"key2":"value2","key1":"value1"}}'),
    ("$(format-json test.* --upper)\n", r'{"TEST":{"KEY2":"value2","KEY1":"value1"}}'),
    ("$(format-json MESSAGE --lower)\n", r'{"message":"-- Generated message. --"}'),
]


@pytest.mark.parametrize(
    "template, expected_value", test_parameters,
    ids=list(map(lambda x: x[0], test_parameters)),
)
def test_value_pairs(config, syslog_ng, template, expected_value):

    generator_source = config.create_example_msg_generator_source(num=1, values="test.key1 => value1 test.key2 => value2")
    file_destination = config.create_file_destination(file_name="output.log", template=config.stringify(template))

    config.create_logpath(statements=[generator_source, file_destination])
    syslog_ng.start(config)
    log = file_destination.read_log()
    assert log == expected_value
