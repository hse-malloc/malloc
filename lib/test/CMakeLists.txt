add_executable(malloc_test
	src/main.cpp
)
target_compile_features(malloc_test PRIVATE cxx_std_20)
target_link_libraries(malloc_test
	PRIVATE
		malloc
)

add_test(NAME malloc_test COMMAND malloc_test)
