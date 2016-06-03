cmake_minimum_required(VERSION 3.0)

project(osm_input)

set(BIN_NAME "osm_input")

include_directories(${PROJECT_SOURCE_DIR}/src)


FILE(GLOB SOURCES_CPP ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wsign-compare -Wunused-variable -Wconversion -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto -ffat-lto-objects -O3 -march=native")

if (CMAKE_BUILD_TYPE MATCHES Debug)
	message("Compilation mode: Debug")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
	message("Compilation mode: Release")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
endif()

add_executable(${BIN_NAME} ${SOURCES_CPP})