if(NOT ENABLE_CPP)
  return()
endif()

# C++ code

set(OTEL_LOGRECORD_CPP_SOURCES
  object-otel-base.hpp
  object-otel-base.cpp
  object-otel.h
  object-otel-logrecord.cpp
  object-otel-logrecord.hpp
  object-otel-resource.cpp
  object-otel-resource.hpp
  object-otel-scope.cpp
  object-otel-scope.hpp
  object-otel-kvlist.cpp
  object-otel-kvlist.hpp
  object-otel-array.cpp
  object-otel-array.hpp
  otel-field-converter.hpp
  otel-field-converter.cpp
)

add_module(
  TARGET otel_filterx_logrecord_cpp
  SOURCES ${OTEL_LOGRECORD_CPP_SOURCES}
  DEPENDS ${MODULE_GRPC_LIBS} grpc-protos grpc-common-cpp
  INCLUDES ${OTEL_PROTO_BUILDDIR} ${PROJECT_SOURCE_DIR}/modules/grpc
  LIBRARY_TYPE STATIC
)

set_target_properties(otel_filterx_logrecord_cpp PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
