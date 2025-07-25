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


include(CMakeParseArguments)

function (add_module)

  cmake_parse_arguments(ADD_MODULE "" "TARGET" "GRAMMAR;GRAMMAR_DEPENDS;SOURCES;DEPENDS;INCLUDES;LIBRARY_TYPE;COMPILE_OPTIONS" ${ARGN})

  if (ADD_MODULE_GRAMMAR)
    module_generate_y_from_ym(${CMAKE_CURRENT_SOURCE_DIR}/${ADD_MODULE_GRAMMAR} ${CMAKE_CURRENT_BINARY_DIR}/${ADD_MODULE_GRAMMAR} ${ADD_MODULE_GRAMMAR_DEPENDS})
    bison_target(${ADD_MODULE_TARGET}Grammar}
      ${CMAKE_CURRENT_BINARY_DIR}/${ADD_MODULE_GRAMMAR}.y
      ${CMAKE_CURRENT_BINARY_DIR}/${ADD_MODULE_GRAMMAR}.c
      COMPILE_FLAGS ${BISON_FLAGS}
      )
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${ADD_MODULE_GRAMMAR}.c
      PROPERTIES
      COMPILE_FLAGS ${BISON_BUILT_SOURCE_CFLAGS}
    )
    list(APPEND ADD_MODULE_SOURCES
         ${CMAKE_CURRENT_BINARY_DIR}/${ADD_MODULE_GRAMMAR}.h
         ${CMAKE_CURRENT_BINARY_DIR}/${ADD_MODULE_GRAMMAR}.c
      )
  endif()

  if (NOT ADD_MODULE_LIBRARY_TYPE)
    set(ADD_MODULE_LIBRARY_TYPE SHARED)
  endif()

  add_library(${ADD_MODULE_TARGET} ${ADD_MODULE_LIBRARY_TYPE} ${ADD_MODULE_SOURCES})
  target_include_directories(${ADD_MODULE_TARGET} SYSTEM PRIVATE ${ADD_MODULE_INCLUDES})
  target_include_directories(${ADD_MODULE_TARGET}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_compile_options(${ADD_MODULE_TARGET} PRIVATE ${ADD_MODULE_COMPILE_OPTIONS})
  target_link_libraries(${ADD_MODULE_TARGET} PRIVATE ${ADD_MODULE_DEPENDS} syslog-ng)

  if (ADD_MODULE_LIBRARY_TYPE STREQUAL SHARED)
    install(TARGETS ${ADD_MODULE_TARGET} LIBRARY DESTINATION lib/syslog-ng COMPONENT ${ADD_MODULE_TARGET})
  endif()

endfunction ()
