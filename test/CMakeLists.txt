add_executable(malloc_test
	src/main.cpp
)
target_compile_features(malloc_test PRIVATE cxx_std_20)

target_link_libraries(malloc_test PRIVATE malloc)

set_target_properties(malloc_test PROPERTIES CXX_EXTENSIONS OFF)

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

#add_test(NAME malloc_test COMMAND malloc_test)
include(CTest)
include(Catch)
catch_discover_tests(malloc_test)
