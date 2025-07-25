#############################################################################
# Copyright (c) 2017 Balabit
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


include(CMakeParseArguments)

if (BUILD_TESTING)
  include(CheckPIESupported)
  check_pie_supported()
endif()

function (add_unit_test)

  if (NOT BUILD_TESTING)
    return()
  endif()

  cmake_parse_arguments(ADD_UNIT_TEST "CRITERION;LIBTEST" "TARGET" "SOURCES;DEPENDS;INCLUDES" ${ARGN})

  if (NOT ADD_UNIT_TEST_SOURCES)
    set(ADD_UNIT_TEST_SOURCES "${ADD_UNIT_TEST_TARGET}.c")
  endif()

  add_executable(${ADD_UNIT_TEST_TARGET} ${ADD_UNIT_TEST_SOURCES})
  target_compile_definitions(${ADD_UNIT_TEST_TARGET} PRIVATE TOP_SRCDIR="${PROJECT_SOURCE_DIR}")
  target_link_libraries(${ADD_UNIT_TEST_TARGET} ${ADD_UNIT_TEST_DEPENDS} syslog-ng)
  target_include_directories(${ADD_UNIT_TEST_TARGET} PUBLIC ${ADD_UNIT_TEST_INCLUDES})
  if (NOT APPLE)
    set_property(TARGET ${ADD_UNIT_TEST_TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--no-as-needed")
  endif()

  if (${ADD_UNIT_TEST_CRITERION})
    target_link_libraries(${ADD_UNIT_TEST_TARGET} ${CRITERION_LIBRARIES})
    target_include_directories(${ADD_UNIT_TEST_TARGET} PUBLIC ${CRITERION_INCLUDE_DIRS})
    set_property(TARGET ${ADD_UNIT_TEST_TARGET} PROPERTY POSITION_INDEPENDENT_CODE FALSE)

  endif()

  if (${ADD_UNIT_TEST_LIBTEST})
    target_link_libraries(${ADD_UNIT_TEST_TARGET} libtest)
  endif()

  foreach(DEPENDENCY ${ADD_UNIT_TEST_DEPENDS})
    if (NOT TARGET ${DEPENDENCY})
      continue()
    endif()

    get_target_property(DEPENDENCY_TYPE ${DEPENDENCY} TYPE)
    if (NOT ${DEPENDENCY_TYPE} STREQUAL "SHARED_LIBRARY")
      continue()
    endif()

    get_property(DEPENDENCY_COMPILE_DEFINITIONS TARGET ${DEPENDENCY} PROPERTY COMPILE_DEFINITIONS)
    target_compile_definitions(${ADD_UNIT_TEST_TARGET} PRIVATE ${DEPENDENCY_COMPILE_DEFINITIONS})
  endforeach()

  add_test (${ADD_UNIT_TEST_TARGET} ${ADD_UNIT_TEST_TARGET})
  add_dependencies(check ${ADD_UNIT_TEST_TARGET})
  set_tests_properties(${ADD_UNIT_TEST_TARGET} PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_odr_violation=0;CRITERION_TEST_PATTERN=!(*/*performance*)")
  set_tests_properties(${ADD_UNIT_TEST_TARGET} PROPERTIES FAIL_REGULAR_EXPRESSION "ERROR: (LeakSanitizer|AddressSanitizer)")
endfunction ()

macro (add_test_subdirectory SUBDIR)
  if (BUILD_TESTING)
    add_subdirectory(${SUBDIR})
  endif()
endmacro()
