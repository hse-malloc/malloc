cmake_minimum_required(VERSION 3.12)

set(NAMESPACE malloc)
set(COMPONENT lib)
project(${NAMESPACE}-${COMPONENT}
        VERSION 0.0.0
        DESCRIPTION "malloc implemetation"
        LANGUAGES CXX
)

# sometimes static is better
add_library(${PROJECT_NAME} SHARED
    include/malloc.hpp
    src/malloc.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME} PRIVATE "include")


