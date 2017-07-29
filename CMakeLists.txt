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

SET(exception_tracing_SOURCES
	${SRC_ROOT}/exception_tracing.cpp
)

SET(stack_tracing_SOURCES
	${SRC_ROOT}/stack_tracing.cpp
)

SET(instrumented_dso_demo_SOURCES
	${SRC_ROOT}/instrumented_dso_demo.cpp
)


# Header file listings


# Target definition

ADD_EXECUTABLE(exception_tracing ${exception_tracing_SOURCES})
ADD_EXECUTABLE(stack_tracing ${stack_tracing_SOURCES})
ADD_LIBRARY(instrumented_dso_demo SHARED ${instrumented_dso_demo_SOURCES})

SET_TARGET_PROPERTIES(exception_tracing PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(stack_tracing PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

SET_TARGET_PROPERTIES(instrumented_dso_demo PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
)

INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/include)

LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
	${CMAKE_INSTALL_PREFIX}/lib
	${CMAKE_INSTALL_PREFIX}/lib/modules/libinstrument
)

TARGET_LINK_LIBRARIES(instrumented_dso_demo instrument)

TARGET_LINK_LIBRARIES(exception_tracing instrument dl bfd pthread instrumented_dso_demo)

TARGET_LINK_LIBRARIES(stack_tracing instrument dl bfd pthread instrumented_dso_demo)


# Set definitions (defines through -D options), -f options, generic options, -W options

ADD_CXX_DEFINITIONS(exception_tracing)
ADD_CXX_FUNCTION_FLAGS(exception_tracing)
ADD_CXX_GENERIC_OPTIONS(exception_tracing)
ADD_CXX_WARNING_FLAGS(exception_tracing)

ADD_CXX_DEFINITIONS(stack_tracing)
ADD_CXX_FUNCTION_FLAGS(stack_tracing)
ADD_CXX_GENERIC_OPTIONS(stack_tracing)
ADD_CXX_WARNING_FLAGS(stack_tracing)

ADD_CXX_DEFINITIONS(instrumented_dso_demo)
ADD_CXX_FUNCTION_FLAGS(instrumented_dso_demo)
ADD_CXX_GENERIC_OPTIONS(instrumented_dso_demo)
ADD_CXX_WARNING_FLAGS(instrumented_dso_demo)


# Configuration rules

#CONFIGURE_FILE(${HDR_IN_ROOT}/config_globals.hpp.in ${PROJECT_SOURCE_DIR}/${HDR_ROOT}/config/config_globals.hpp)


# Installation rules

#INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

#INSTALL(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})


# Store final variable set

LIST_VARIABLES(${PROJECT_BINARY_DIR}/${PROJECT_NAME}_variables.txt)
