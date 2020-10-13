cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(malloc_example
	VERSION 1.0.0
	DESCRIPTION "example project using malloc()"
	LANGUAGES CXX)

add_executable(${PROJECT_NAME}
	src/main.cpp
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

find_package(Malloc REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Malloc::Malloc)

