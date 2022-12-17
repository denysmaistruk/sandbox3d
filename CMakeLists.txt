cmake_minimum_required(VERSION 3.18)

project(physbox VERSION 0.1)

# project config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "physbox")

add_subdirectory(extern/raylib)
add_subdirectory(extern/imgui)
add_subdirectory(extern/cyclone)

list(APPEND EXTRA_LIBS raylib
                       imgui
                       cyclone
                       )
					   
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/src"
                           "${PROJECT_SOURCE_DIR}/extern/raylib/src"
                           "${PROJECT_SOURCE_DIR}/extern/imgui/imgui"
                           "${PROJECT_SOURCE_DIR}/extern/cyclone/cyclone/include"
			   )

file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp
                               ${PROJECT_SOURCE_DIR}/src/*.h
                               ${PROJECT_SOURCE_DIR}/src/*.vs
                               ${PROJECT_SOURCE_DIR}/src/*.fs
                               )

list(APPEND SOURCE_FILES ${PROJECT_SOURCE_DIR}/main.cpp )

add_executable(physbox ${SOURCE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

target_precompile_headers(physbox PUBLIC "${PROJECT_SOURCE_DIR}/src/pch.h")

target_link_libraries(physbox PUBLIC ${EXTRA_LIBS})

target_include_directories(physbox PUBLIC ${EXTRA_INCLUDES})
