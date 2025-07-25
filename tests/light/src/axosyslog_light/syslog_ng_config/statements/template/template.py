#!/usr/bin/env python
#############################################################################
# Copyright (c) 2023 Balázs Scheidler <balazs.scheidler@axoflow.com>
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
from axosyslog_light.common.random_id import get_unique_id


class Template(object):
    def __init__(self, template, name=None, escape=None, use_simple_statement=False):
        self.template = template
        self.template_escape = escape
        self.name = name or "template_{}".format(get_unique_id())
        self.use_simple_statement = escape is not None or use_simple_statement


class TemplateFunction(object):
    def __init__(self, template, name):
        self.template = template
        self.name = name
