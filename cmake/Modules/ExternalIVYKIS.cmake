#############################################################################
# Copyright (c) 2017 Balabit
# Copyright (c) 2017 Kokan
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

set(LIB_NAME "IVYKIS")

if (EXISTS ${PROJECT_SOURCE_DIR}/lib/ivykis/src/include/iv.h.in)

    ExternalProject_Add(
        ${LIB_NAME}
        PREFIX            ${CMAKE_CURRENT_BINARY_DIR}/ivykis-install/
        INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/ivykis-install/
        SOURCE_DIR        ${PROJECT_SOURCE_DIR}/lib/ivykis/
        DOWNLOAD_COMMAND  echo
        BUILD_COMMAND     make
        INSTALL_COMMAND   make install
        CONFIGURE_COMMAND autoreconf -i ${PROJECT_SOURCE_DIR}/lib/ivykis && ${PROJECT_SOURCE_DIR}/lib/ivykis/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/ivykis-install/
    )

    set(${LIB_NAME}_INTERNAL TRUE)
    set(${LIB_NAME}_INTERNAL_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ivykis-install/include" CACHE STRING "${LIB_NAME} include path")
    set(${LIB_NAME}_INTERNAL_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/ivykis-install/lib/libivykis${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE STRING "${LIB_NAME} library path")
    set(SYSLOG_NG_HAVE_IV_WORK_POOL_SUBMIT_CONTINUATION 1 PARENT_SCOPE)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ivykis-install/lib/ DESTINATION lib USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "libivykis${CMAKE_SHARED_LIBRARY_SUFFIX}*")

else()
  set(${LIB_NAME}_INTERNAL FALSE)
endif()

