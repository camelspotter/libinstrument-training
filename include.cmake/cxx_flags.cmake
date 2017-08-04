CMAKE_MINIMUM_REQUIRED(VERSION 3.8)


# Add definitions to a target

FUNCTION(ADD_CXX_DEFINITIONS TARGET_ARG)

	GET_PROPERTY(OPTS DIRECTORY ./ PROPERTY VARIABLES)

	FOREACH(OPT ${OPTS})

		IF("${OPT}" MATCHES "WITH_" AND ${OPT})

			TARGET_COMPILE_DEFINITIONS(${TARGET_ARG} PUBLIC ${OPT})

		ENDIF("${OPT}" MATCHES "WITH_" AND ${OPT})

	ENDFOREACH(OPT)

ENDFUNCTION(ADD_CXX_DEFINITIONS)


# Add -f family flags to a target

FUNCTION(ADD_CXX_FUNCTION_FLAGS TARGET_ARG)

	TARGET_COMPILE_OPTIONS(${TARGET_ARG} PUBLIC
		-fno-enforce-eh-specs

		-fPIC

		-fstrict-aliasing
	)

ENDFUNCTION(ADD_CXX_FUNCTION_FLAGS)


# Add -finstrument family flags to a target

FUNCTION(ADD_CXX_INSTRUMENTATION_FLAGS TARGET_ARG)

	TARGET_COMPILE_OPTIONS(${TARGET_ARG} PUBLIC
		-finstrument-functions

		-finstrument-functions-exclude-file-list=/usr/include

		-finstrument-functions-exclude-file-list=iostream

		-finstrument-functions-exclude-file-list=ios

		-finstrument-functions-exclude-file-list=istream

		-finstrument-functions-exclude-file-list=ostream

		-finstrument-functions-exclude-file-list=${CMAKE_INSTALL_PREFIX}/include/instrument
	)

ENDFUNCTION(ADD_CXX_INSTRUMENTATION_FLAGS)


# Add generic options to a target

FUNCTION(ADD_CXX_GENERIC_OPTIONS TARGET_ARG)

	TARGET_COMPILE_OPTIONS(${TARGET_ARG} PUBLIC
		-O2

		-g

		-march=native

		-rdynamic

		-std=gnu++0x
	)

ENDFUNCTION(ADD_CXX_GENERIC_OPTIONS)


# Add -W family flags to a target

FUNCTION(ADD_CXX_WARNING_FLAGS TARGET_ARG)

	TARGET_COMPILE_OPTIONS(${TARGET_ARG} PUBLIC
		-Wall

		-Wabi

		-Wcast-align

		-Wcast-qual

		-Wclobbered

		-Wctor-dtor-privacy

		-Wdisabled-optimization

		-Wempty-body

		-Wformat-security

		-Winit-self

		-Wlogical-op

		-Wmissing-field-initializers

		-Wmissing-include-dirs

		-Wmissing-noreturn

		-Wnon-virtual-dtor

		-Woverlength-strings

		-Wpacked

		-Wredundant-decls

		-Wsign-compare

		-Wswitch-enum

		-Wtype-limits
	)

ENDFUNCTION(ADD_CXX_WARNING_FLAGS)
