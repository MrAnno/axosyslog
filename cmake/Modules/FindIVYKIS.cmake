#############################################################################
# Copyright (c) 2016 Balabit
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

include(LibFindMacros)

libfind_pkg_check_modules(IVYKIS_PKGCONF ivykis)

libfind_pkg_detect(IVYKIS ivykis FIND_PATH iv.h FIND_LIBRARY ivykis)
set(IVYKIS_PROCESS_INCLUDES IVYKIS_INCLUDE_DIR)
set(IVYKIS_PROCESS_LIBS IVYKIS_LIBRARY)
libfind_process(IVYKIS)

check_symbol_exists(iv_work_pool_submit_continuation "iv_work.h" SYSLOG_NG_HAVE_IV_WORK_POOL_SUBMIT_CONTINUATION)
set(SYSLOG_NG_HAVE_IV_WORK_POOL_SUBMIT_CONTINUATION "${SYSLOG_NG_HAVE_IV_WORK_POOL_SUBMIT_CONTINUATION}" PARENT_SCOPE)
