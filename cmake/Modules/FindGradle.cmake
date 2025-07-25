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

include(FindPackageHandleStandardArgs)

find_program(GRADLE_EXECUTABLE NAMES gradle PATHS $ENV{GRADLE_HOME}/bin NO_CMAKE_FIND_ROOT_PATH)

if (GRADLE_EXECUTABLE)
    execute_process(COMMAND gradle --version
                    COMMAND grep Gradle
                    COMMAND head -1
                    COMMAND sed "s/.*\\ \\(.*\\)/\\1/"
                    OUTPUT_VARIABLE GRADLE_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
endif()

find_package_handle_standard_args(Gradle FOUND_VAR GRADLE_FOUND  REQUIRED_VARS GRADLE_EXECUTABLE VERSION_VAR GRADLE_VERSION)
mark_as_advanced(GRADLE_EXECUTABLE)
