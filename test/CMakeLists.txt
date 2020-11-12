add_executable(malloc_test
	src/main.cpp
)
target_compile_features(malloc_test PRIVATE cxx_std_20)

target_link_libraries(malloc_test PRIVATE malloc-implementation)

target_include_directories(malloc_test
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(malloc_test PRIVATE
        $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
                -Wall
                -Wextra
                -Wpedantic
        >
        PUBLIC
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:
        	$<$<CONFIG:DEBUG>:
        		-fprofile-instr-generate
        		-fcoverage-mapping
        	>
        >)

find_package(Catch2)
if(NOT Catch2_FOUND)
    Include(FetchContent)

    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v2.13.1)

    FetchContent_MakeAvailable(Catch2)
endif()

include(CTest)
include(Catch)
catch_discover_tests(malloc_test)
