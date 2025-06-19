#!/usr/bin/env python
#############################################################################
# Copyright (c) 2020 Balabit
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


def test_set_tag(config, syslog_ng):
    match = config.create_match_filter(match_string=config.stringify("MATCHSTRING"), value=config.stringify("MSG"))
    notmatch = config.create_match_filter(match_string=config.stringify("NONE"), value=config.stringify("MSG"))

    generator_source = config.create_example_msg_generator_source(num=1, template=config.stringify("input with MATCHSTRING in it"))
    set_tag_with_matching = config.create_rewrite_set_tag(config.stringify("SHOULDMATCH"), condition=match)
    set_tag_without_matching = config.create_rewrite_set_tag(config.stringify("DONOTMATCH"), condition=notmatch)
    file_destination = config.create_file_destination(file_name="output.log", template=config.stringify('${TAGS}\n'))
    config.create_logpath(statements=[generator_source, set_tag_with_matching, set_tag_without_matching, file_destination])

    syslog_ng.start(config)
    log_line = file_destination.read_log().strip()
    assert "SHOULDMATCH" in log_line
    assert "DONOTMATCH" not in log_line


def test_set_tag_with_template(config, syslog_ng):
    generator_source = config.create_example_msg_generator_source(num=1, template=config.stringify("FOO"))
    set_tag_with_template = config.create_rewrite_set_tag(config.stringify("TAG-${MSG}"))
    file_destination = config.create_file_destination(file_name="output.log", template=config.stringify('${TAGS}\n'))
    config.create_logpath(statements=[generator_source, set_tag_with_template, file_destination])

    syslog_ng.start(config)
    log_line = file_destination.read_log().strip()
    assert "TAG-FOO" in log_line
