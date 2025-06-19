#############################################################################
# Copyright (c) 2022 One Identity
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

from inspect import isclass
from typing import Dict, Type

from .cdn import CDN
from .azure_cdn import AzureCDN
from .cloudflare_cdn import CloudflareCDN

__all__ = [
    "CDN",
    "AzureCDN",
    "CloudflareCDN",
    "get_implementations",
]


def get_implementations() -> Dict[str, Type[CDN]]:
    implementations = {}

    for cls_name in __all__:
        cls = globals()[cls_name]
        if isclass(cls) and cls != CDN and issubclass(cls, CDN):
            implementations[cls.get_config_keyword()] = cls

    return implementations
