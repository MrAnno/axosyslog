module_switch(ENABLE_XML "Enable xml")

if (NOT ENABLE_XML)
  return()
endif()

set(xml_SOURCES
    "xml-parser.h"
    "xml.h"
    "xml-private.h"
    "windows-eventlog-xml-parser.h"
    "filterx-parse-xml.h"
    "filterx-parse-windows-eventlog-xml.h"
    "filterx-func-format-xml.h"
    "filterx-func-format-windows-eventlog-xml.h"

    "xml-plugin.c"
    "xml-parser.c"
    "xml.c"
    "windows-eventlog-xml-parser.c"
    "filterx-parse-xml.c"
    "filterx-parse-windows-eventlog-xml.c"
    "filterx-func-format-xml.c"
    "filterx-func-format-windows-eventlog-xml.c"
)


add_module(
  TARGET xml
  GRAMMAR xml-grammar
  SOURCES ${xml_SOURCES}
)

add_test_subdirectory(tests)
