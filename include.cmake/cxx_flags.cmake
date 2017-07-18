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
		-finstrument-functions

		-finstrument-functions-exclude-file-list=/usr/include

		-finstrument-functions-exclude-file-list=iostream

		-finstrument-functions-exclude-file-list=ios

		-finstrument-functions-exclude-file-list=istream

		-finstrument-functions-exclude-file-list=ostream

		-finstrument-functions-exclude-file-list=${CMAKE_INSTALL_PREFIX}/include/instrument

		-fno-enforce-eh-specs

		-fPIC

		-fstrict-aliasing
	)

ENDFUNCTION(ADD_CXX_FUNCTION_FLAGS)
