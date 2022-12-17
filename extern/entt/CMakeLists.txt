cmake_minimum_required(VERSION 3.18)

project(entt VERSION 3.11.1)

set(CMAKE_CXX_STANDARD 17)

list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/entt/single_include/entt/")
						   
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/entt/single_include/entt/*.hpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
					   )

add_library(entt ${SOURCE_FILES})

target_include_directories(entt PUBLIC ${EXTRA_INCLUDES})