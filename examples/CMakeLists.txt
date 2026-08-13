# Equinox-Logger 2.1.1
# Author: Janusz Wolak
# Copyright (C) 2026

cmake_minimum_required(VERSION 3.22.1)
project(EquinoxLoggerExamples)

message(STATUS "Processing CMakeLists.txt for: " ${PROJECT_NAME})
message(STATUS "CMAKE_SOURCE_DIR:	" ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EQUINOX_LOGGER_LIB_DIR ${PROJECT_SOURCE_DIR}/../libEquinoxLogger.so)

set(EQUINOX_LOGGER_API_HEADER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../)

set(EQUINOX_LOGGER_EXAMPLER_SRC_DIR
    ${PROJECT_SOURCE_DIR}/src
)

set(EQUINOX_LOGGER_EXAMPLES_SRC
    ${EQUINOX_LOGGER_EXAMPLER_SRC_DIR}/EquinoxLoggerExamples.cpp
)

add_executable(${PROJECT_NAME} ${EQUINOX_LOGGER_EXAMPLES_SRC})
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/..
        ${PROJECT_SOURCE_DIR}/../api
        ${PROJECT_SOURCE_DIR}/../include
)
add_dependencies (${PROJECT_NAME} EquinoxLogger)
target_link_libraries(${PROJECT_NAME} EquinoxLogger)