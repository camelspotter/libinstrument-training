CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

INCLUDE(include.cmake/util.cmake)

INCLUDE(include.cmake/eclipse.cmake)

INCLUDE(include.cmake/cxx_flags.cmake)


# Environment variables

SET(CMAKE_C_COMPILER /usr/bin/gcc CACHE FILEPATH "C compiler" FORCE)

SET(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE FILEPATH "C++ compiler" FORCE)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "compiler invocations" FORCE)

SET(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "make output enable" FORCE)


# Project definition

PROJECT(libinstrument-training C CXX)

DESCRIBE_HOST()

DESCRIBE_TARGET()

IS_SUPPORTED()

SET(${PROJECT_NAME}_VERSION_MAJOR 1)

SET(${PROJECT_NAME}_VERSION_MINOR 0)

SET(CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "installation prefix" FORCE)


# Fixed primary option definitions

# Dynamic option definitions


# Source directory listings

SET(SRC_ROOT src)


# Source file listings

SET(aop_interceptor_SOURCES
	${SRC_ROOT}/aop_interceptor.cpp
)

SET(exception_tracing_SOURCES
	${SRC_ROOT}/exception_tracing.cpp
)

SET(stack_tracing_SOURCES
	${SRC_ROOT}/stack_tracing.cpp
)

SET(trace_logging_SOURCES
	${SRC_ROOT}/trace_logging.cpp
)

SET(instrumented_dso_demo_SOURCES
	${SRC_ROOT}/instrumented_dso_demo.cpp
)

SET(mod_test_SOURCES
	${SRC_ROOT}/mod_test.cpp
)

SET(mod_callgraph_SOURCES
	${SRC_ROOT}/mod_callgraph.cpp
)


# Header file listings


# Target definition

ADD_EXECUTABLE(aop_interceptor ${aop_interceptor_SOURCES})
ADD_EXECUTABLE(exception_tracing ${exception_tracing_SOURCES})
ADD_EXECUTABLE(stack_tracing ${stack_tracing_SOURCES})
ADD_EXECUTABLE(trace_logging ${trace_logging_SOURCES})

ADD_LIBRARY(instrumented_dso_demo SHARED ${instrumented_dso_demo_SOURCES})
ADD_LIBRARY(mod_test SHARED ${mod_test_SOURCES})
ADD_LIBRARY(mod_callgraph SHARED ${mod_callgraph_SOURCES})


SET_TARGET_PROPERTIES(aop_interceptor PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(exception_tracing PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(stack_tracing PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(trace_logging PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(instrumented_dso_demo PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(mod_test PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(mod_callgraph PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)


INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/include)

LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
	${CMAKE_INSTALL_PREFIX}/lib
	${CMAKE_INSTALL_PREFIX}/lib/modules/libinstrument
)


TARGET_LINK_LIBRARIES(instrumented_dso_demo instrument)

TARGET_LINK_LIBRARIES(mod_test instrument)

TARGET_LINK_LIBRARIES(mod_callgraph instrument)

TARGET_LINK_LIBRARIES(aop_interceptor instrument dl bfd pthread instrumented_dso_demo)

TARGET_LINK_LIBRARIES(exception_tracing instrument dl bfd pthread instrumented_dso_demo)

TARGET_LINK_LIBRARIES(stack_tracing instrument dl bfd pthread instrumented_dso_demo)

TARGET_LINK_LIBRARIES(trace_logging instrument dl bfd pthread instrumented_dso_demo)


# Set definitions (defines through -D options), -f options, generic options, -W options

ADD_CXX_DEFINITIONS(aop_interceptor)
ADD_CXX_FUNCTION_FLAGS(aop_interceptor)
ADD_CXX_INSTRUMENTATION_FLAGS(aop_interceptor)
ADD_CXX_GENERIC_OPTIONS(aop_interceptor)
ADD_CXX_WARNING_FLAGS(aop_interceptor)

ADD_CXX_DEFINITIONS(exception_tracing)
ADD_CXX_FUNCTION_FLAGS(exception_tracing)
ADD_CXX_INSTRUMENTATION_FLAGS(exception_tracing)
ADD_CXX_GENERIC_OPTIONS(exception_tracing)
ADD_CXX_WARNING_FLAGS(exception_tracing)

ADD_CXX_DEFINITIONS(stack_tracing)
ADD_CXX_FUNCTION_FLAGS(stack_tracing)
ADD_CXX_INSTRUMENTATION_FLAGS(stack_tracing)
ADD_CXX_GENERIC_OPTIONS(stack_tracing)
ADD_CXX_WARNING_FLAGS(stack_tracing)

ADD_CXX_DEFINITIONS(trace_logging)
ADD_CXX_FUNCTION_FLAGS(trace_logging)
ADD_CXX_INSTRUMENTATION_FLAGS(trace_logging)
ADD_CXX_GENERIC_OPTIONS(trace_logging)
ADD_CXX_WARNING_FLAGS(trace_logging)

ADD_CXX_DEFINITIONS(instrumented_dso_demo)
ADD_CXX_FUNCTION_FLAGS(instrumented_dso_demo)
ADD_CXX_INSTRUMENTATION_FLAGS(instrumented_dso_demo)
ADD_CXX_GENERIC_OPTIONS(instrumented_dso_demo)
ADD_CXX_WARNING_FLAGS(instrumented_dso_demo)

ADD_CXX_DEFINITIONS(mod_test)
ADD_CXX_FUNCTION_FLAGS(mod_test)
ADD_CXX_GENERIC_OPTIONS(mod_test)
ADD_CXX_WARNING_FLAGS(mod_test)

ADD_CXX_DEFINITIONS(mod_callgraph)
ADD_CXX_FUNCTION_FLAGS(mod_callgraph)
ADD_CXX_GENERIC_OPTIONS(mod_callgraph)
ADD_CXX_WARNING_FLAGS(mod_callgraph)


# Installation rules

INSTALL(TARGETS aop_interceptor RUNTIME DESTINATION bin)

INSTALL(TARGETS exception_tracing RUNTIME DESTINATION bin)

INSTALL(TARGETS stack_tracing RUNTIME DESTINATION bin)

INSTALL(TARGETS trace_logging RUNTIME DESTINATION bin)

INSTALL(TARGETS instrumented_dso_demo LIBRARY DESTINATION lib/modules/libinstrument)

INSTALL(TARGETS mod_test LIBRARY DESTINATION lib/modules/libinstrument)

INSTALL(TARGETS mod_callgraph LIBRARY DESTINATION lib/modules/libinstrument)


# Store final variable set

LIST_VARIABLES(${PROJECT_BINARY_DIR}/${PROJECT_NAME}_variables.txt)


# Install back in sources the installation manifests

INSTALL(FILES ${PROJECT_BINARY_DIR}/compile_commands.json DESTINATION ${PROJECT_SOURCE_DIR}/doc/)

# todo post-install
# INSTALL(FILES ${PROJECT_BINARY_DIR}/install_manifest.txt DESTINATION ${PROJECT_SOURCE_DIR}/doc/)

INSTALL(FILES ${PROJECT_BINARY_DIR}/libinstrument-training_variables.txt DESTINATION ${PROJECT_SOURCE_DIR}/doc/)
