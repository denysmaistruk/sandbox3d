cmake_minimum_required(VERSION 3.18)

project(sandbox3d VERSION 0.01)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "sandbox3d")

add_subdirectory(extern/raylib)
add_subdirectory(extern/imgui)
add_subdirectory(extern/cyclone)
add_subdirectory(extern/imguizmo)
add_subdirectory(extern/entt)

set_target_properties(raylib   PROPERTIES FOLDER "External")
set_target_properties(imgui    PROPERTIES FOLDER "External")
set_target_properties(cyclone  PROPERTIES FOLDER "External")
set_target_properties(imguizmo PROPERTIES FOLDER "External")
set_target_properties(entt 	   PROPERTIES FOLDER "External")

list(APPEND EXTRA_LIBS raylib
                       imgui
                       cyclone
                       imguizmo
                       entt
                       )
					   
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/src"
                           "${PROJECT_SOURCE_DIR}/extern/raylib/src"
                           "${PROJECT_SOURCE_DIR}/extern/imgui/imgui"
                           "${PROJECT_SOURCE_DIR}/extern/cyclone/cyclone/include"
                           "${PROJECT_SOURCE_DIR}/extern/imguizmo/imguizmo"
						   "${PROJECT_SOURCE_DIR}/extern/entt"
							)

file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp
                               ${PROJECT_SOURCE_DIR}/src/*.h
                               ${PROJECT_SOURCE_DIR}/src/*.vs
                               ${PROJECT_SOURCE_DIR}/src/*.fs
                               )

list(APPEND SOURCE_FILES ${PROJECT_SOURCE_DIR}/main.cpp )

add_executable(sandbox3d ${SOURCE_FILES})

target_compile_options(sandbox3d PUBLIC "/FIsandbox3d.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

target_precompile_headers(sandbox3d PUBLIC "${PROJECT_SOURCE_DIR}/src/pch.h")

target_link_libraries(sandbox3d PUBLIC ${EXTRA_LIBS})

target_include_directories(sandbox3d PUBLIC ${EXTRA_INCLUDES})