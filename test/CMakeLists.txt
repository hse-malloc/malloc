add_executable(${PROJECT_NAME}_test
	src/main.cpp
)
target_compile_features(${PROJECT_NAME}_test PRIVATE cxx_std_20)

target_link_libraries(${PROJECT_NAME}_test PRIVATE hse_${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}_test
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(${PROJECT_NAME}_test
	PRIVATE
	$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
#		-Wall
#		-Wextra
#		-Wpedantic
	>
	PUBLIC
	$<$<CXX_COMPILER_ID:Clang,AppleClang>:
		$<$<CONFIG:DEBUG>:
			-fprofile-instr-generate
			-fcoverage-mapping
		>
	>
)

find_package(Catch2 QUIET)
if(NOT Catch2_FOUND)
    Include(FetchContent)

    FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG        v2.13.3
        )

    FetchContent_MakeAvailable(Catch2)
endif()

target_link_libraries(${PROJECT_NAME}_test PRIVATE Catch2::Catch2)

add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

