cmake_minimum_required(VERSION 3.18)

project(cyclone VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/cyclone/include")
						   
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/cyclone/src/*.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/cyclone/include/cyclone/*.h
					   )
add_library(cyclone ${SOURCE_FILES})

target_compile_options(cyclone PUBLIC "/W0")

target_include_directories(cyclone PUBLIC ${EXTRA_INCLUDES})