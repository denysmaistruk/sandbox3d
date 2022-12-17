cmake_minimum_required(VERSION 3.18)

project(imguizmo VERSION 1.83)

set(CMAKE_CXX_STANDARD 17)

list(APPEND EXTRA_INCLUDES "${CMAKE_SOURCE_DIR}/extern/imgui/imgui")
						   
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/imguizmo/*.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/imguizmo/*.h
					   )

#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(imguizmo ${SOURCE_FILES})

target_include_directories(imguizmo PUBLIC ${EXTRA_INCLUDES})