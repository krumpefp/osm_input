cmake_minimum_required(VERSION 3.0)

project(osm_input)

find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)

set(BIN_NAME "osm_input")

include_directories(src
                    src/debughelpers
                    src/input
                    src/labelhelpers
                    src/output
                    src/primitives
                    vendor/osmpbf
                    vendor/osmpbf/osmpbf/include)


FILE(GLOB SOURCES_CPP src/*.cpp
                      src/debughelpers/*.cpp
                      src/input/*.cpp
                      src/labelhelpers/*.cpp
                      src/output/*.cpp
                      src/primitives/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11  -fopenmp")
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

set(MY_LINK_LIBS
    ${osmpbf_BINARY_DIR}/libosmpbf.a
    ${PROTOBUF_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

add_executable(${BIN_NAME} ${SOURCES_CPP})

add_subdirectory(vendor/osmpbf)
add_dependencies(${PROJECT_NAME} osmpbf)
target_link_libraries(${BIN_NAME} ${MY_LINK_LIBS})
