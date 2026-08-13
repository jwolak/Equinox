# Equinox-Logger 2.1.1
# Author: Janusz Wolak
# Copyright (C) 2026

#------------------------------------------------------------------------------------------
#                                cmake settings
#------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.22.1)

#------------------------------------------------------------------------------------------
#                                Project settings
#------------------------------------------------------------------------------------------
project(EquinoxLogger)

set(LIB_MAJOR_VERSION "2")
set(LIB_MINOR_VERSION "1")
set(LIB_PATCH_VERSION "1")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

message(STATUS "Processing CMakeLists.txt for: " ${PROJECT_NAME} " " ${LIB_VERSION_STRING})
message(STATUS "CMAKE_SOURCE_DIR:	" ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#------------------------------------------------------------------------------------------
#                                Project options
#------------------------------------------------------------------------------------------

option(EQUINOX_LOGGER_TESTS         "Build tests"           ON)
option(EQUINOX_LOGGER_EXAMPLES      "Build examples"        ON)
option(EQUINOX_LOGGER_BUILD_SHARED  "Build shared lib"      ON)
option(EQUINOX_LOGGER_BUILD_STATIC  "Build static lib"      OFF)

#------------------------------------------------------------------------------------------
#                                Compiler flags
#------------------------------------------------------------------------------------------

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Build type not specified. Defaulting to Release")
endif()

# Debug flags: Include symbols, no optimization, extra warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pedantic"
    CACHE STRING "Debug C++ compiler flags" FORCE)

# Release flags: Optimize for performance, strip symbols, extra warnings
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -Wall -Wextra"
    CACHE STRING "Release C++ compiler flags" FORCE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler flags (Debug):   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ compiler flags (Release): ${CMAKE_CXX_FLAGS_RELEASE}")

#------------------------------------------------------------------------------------------
#                                Project api
#------------------------------------------------------------------------------------------
set(EQUINOX_LOGGER_API
	${CMAKE_CURRENT_SOURCE_DIR}/api
)

#------------------------------------------------------------------------------------------
#                                Project include
#------------------------------------------------------------------------------------------
set(EQUINOX_LOGGER_INCLUDE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(EQUINOX_LOGGER_FMT_INCLUDE
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt/include
)

set(EQUINOX_LOGGER_FMT_LIB
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt/x64/libfmt.a
)

add_library(fmt STATIC IMPORTED GLOBAL)
set_target_properties(fmt PROPERTIES
	IMPORTED_LOCATION "${EQUINOX_LOGGER_FMT_LIB}"
	INTERFACE_INCLUDE_DIRECTORIES "${EQUINOX_LOGGER_FMT_INCLUDE}"
)

#------------------------------------------------------------------------------------------
#                                Project source
#------------------------------------------------------------------------------------------
set(EQUINOX_LOGGER_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/EquinoxLoggerEngine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/EquinoxLoggerEngineImpl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/EquinoxLogger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/AsyncLogQueue.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/TimestampProducer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleLogsProducer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/FileLogsProducer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/AsyncLogQueueEngine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ColorFormatter.cpp
)

#------------------------------------------------------------------------------------------
#                                Project target
#------------------------------------------------------------------------------------------
if (EQUINOX_LOGGER_BUILD_SHARED)
	add_library(EquinoxLogger SHARED ${EQUINOX_LOGGER_SRC})
	target_include_directories(EquinoxLogger
		PUBLIC
			${EQUINOX_LOGGER_INCLUDE}
			${EQUINOX_LOGGER_API}
			${EQUINOX_LOGGER_FMT_INCLUDE}
		PRIVATE
			${EQUINOX_LOGGER_INCLUDE}
			${EQUINOX_LOGGER_API}
	)
	target_link_libraries(EquinoxLogger PUBLIC fmt)
	target_compile_definitions(EquinoxLogger PUBLIC EQUINOX_SHARED_SHARED_LIB)
endif()

if (EQUINOX_LOGGER_BUILD_STATIC)
	add_library(EquinoxLogger STATIC ${EQUINOX_LOGGER_SRC})
	target_include_directories(EquinoxLogger
		PUBLIC
			${EQUINOX_LOGGER_INCLUDE}
			${EQUINOX_LOGGER_API}
			${EQUINOX_LOGGER_FMT_INCLUDE}
		PRIVATE
			${EQUINOX_LOGGER_INCLUDE}
			${EQUINOX_LOGGER_API}
	)
	target_link_libraries(EquinoxLogger PUBLIC fmt)
endif()

#------------------------------------------------------------------------------------------
#                                Project tests
#------------------------------------------------------------------------------------------
if(EQUINOX_LOGGER_TESTS)
	add_subdirectory(tests)
endif(EQUINOX_LOGGER_TESTS)

#------------------------------------------------------------------------------------------
#                                Project examples
#------------------------------------------------------------------------------------------
if(EQUINOX_LOGGER_EXAMPLES)
	add_subdirectory(examples)
endif(EQUINOX_LOGGER_EXAMPLES)

#------------------------------------------------------------------------------------------
#                                Project install
#------------------------------------------------------------------------------------------
install(FILES ${EQUINOX_LOGGER_API}/EquinoxLogger.h
              ${EQUINOX_LOGGER_API}/EquinoxLogger.hpp
              ${EQUINOX_LOGGER_API}/EquinoxLoggerCommon.h
              ${EQUINOX_LOGGER_API}/EquinoxLoggerEngine.h
        DESTINATION include)

install(TARGETS EquinoxLogger DESTINATION lib)
install(FILES EquinoxLoggerConfig.cmake DESTINATION lib/cmake/EquinoxLogger)


