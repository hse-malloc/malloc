cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(malloc_example_c
	VERSION 1.0.0
        DESCRIPTION "example c project using malloc()"
        LANGUAGES C)

add_executable(${PROJECT_NAME}
        src/main.c
)

find_package(malloc REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Malloc::Malloc)

