
cmake_minimum_required(VERSION 3.12)

set(NAMESPACE malloc)
set(COMPONENT test)
project(${NAMESPACE}-${COMPONENT}
        VERSION 0.0.0
        DESCRIPTION "Malloc tests"
        LANGUAGES CXX
)

add_executable(${PROJECT_NAME}
    src/run.cpp
)

#TO DO
#find_package(malloc-lib)
# 
#target_link_libraries(${PROJECT_NAME} PRIVATE malloc::lib)
