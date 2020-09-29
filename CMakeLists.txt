
cmake_minimum_required(VERSION 3.12)

project(malloc
        DESCRIPTION "Malloc library with tests"
        VERSION 0.0.0
        LANGUAGES CXX
)

add_subdirectory(malloc-lib)

#TO DO
#add_subdirectory(malloc-test)

include(FeatureSummary)
feature_summary(WHAT ALL)