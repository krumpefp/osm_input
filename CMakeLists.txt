cmake_minimum_required(VERSION 3.0)

project(osm_input)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Cairo REQUIRED)
find_package(Freetype REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS})

include_directories(src
	src/config
	src/debughelpers
	src/input
	src/label
	src/mapping
	src/output
	src/primitives
	src/statistics
	#vendor/argument_parser/src/lib/include
	vendor/jsoncpp/dist/json
	vendor/osmpbf
	vendor/osmpbf/osmpbf/include)


FILE(GLOB SOURCES_CPP src/*.cpp
	src/*.cpp
	src/config/*.cpp
	src/debughelpers/*.cpp
	src/input/*.cpp
	src/label/*.cpp
	src/mapping/*.cpp
	src/output/*.cpp
	src/primitives/*.cpp
	src/statistics/*.cpp)
#vendor/argument_parser/src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11  -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wsign-compare -Wunused-variable -Wconversion -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto -ffat-lto-objects -O3 -march=native")

add_subdirectory(vendor/argument_parser)
add_subdirectory(vendor/jsoncpp)
add_subdirectory(vendor/osmpbf)

set(MY_LINK_LIBS
	argumentparser
	jsoncpp
	osmpbf
	${CAIRO_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${PROTOBUF_LIBRARIES}
	${ZLIB_LIBRARIES}
	)

add_executable(${PROJECT_NAME} ${SOURCES_CPP})

add_dependencies(${PROJECT_NAME} osmpbf)
target_link_libraries(${PROJECT_NAME} ${MY_LINK_LIBS})
