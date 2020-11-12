cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(malloc
	VERSION 1.0
	DESCRIPTION "custom malloc() implementation with randomized addresses"
	LANGUAGES CXX
)

add_library(${PROJECT_NAME}-implementation
        src/malloc/malloc_implementation.cpp
        src/malloc/malloc_implementation.h
	src/system/system.cpp
	src/system/system.h
	src/memory/memory_control_block.cpp
	src/memory/memory_control_block.h
	src/memory/memory_control_block_list.cpp
	src/memory/memory_control_block_list.h
	src/memory/allocator.cpp
	src/memory/allocator.h
	src/random/random.h
	src/math/math.h
)
set_target_properties(${PROJECT_NAME}-implementation PROPERTIES
    CXX_EXTENSIONS OFF
    EXPORT_NAME Malloc-implementation)

add_library(${PROJECT_NAME}
        include/malloc.h
        src/malloc.cpp
        )

add_library(Malloc::Malloc ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_EXTENSIONS OFF
	EXPORT_NAME Malloc
	PUBLIC_HEADER
		include/malloc.h
)


target_compile_definitions(${PROJECT_NAME}-implementation
    PRIVATE "$<$<CONFIG:DEBUG>:HSE_MALLOC_DEBUG>"
    PRIVATE "$<$<PLATFORM_ID:Linux, Darwin>:HAVE_DEV_URANDOM>"
    PRIVATE "$<$<PLATFORM_ID:Linux, Darwin>:HAVE_MMAP>"
    PRIVATE "$<$<PLATFORM_ID:Windows>:HAVE_VIRTUAL_ALLOC>")


target_include_directories(${PROJECT_NAME}-implementation
        PUBLIC
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:include>
        PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(${PROJECT_NAME}-implementation PRIVATE cxx_std_20)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(${PROJECT_NAME}-implementation PRIVATE
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

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
#                -Wall
#                -Wextra
#                -Wpedantic
        >
        PUBLIC
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:
                $<$<CONFIG:DEBUG>:
                        -fprofile-instr-generate
                        -fcoverage-mapping
                >
        >
        )

target_link_options(${PROJECT_NAME}-implementation PUBLIC
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:
        	$<$<CONFIG:DEBUG>:
        		-fprofile-instr-generate
        		-fcoverage-mapping
        	>
        >
    )

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-implementation)


target_link_options(${PROJECT_NAME} PUBLIC
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:
                $<$<CONFIG:DEBUG>:
                        -fprofile-instr-generate
                        -fcoverage-mapping
                >
        >
    )

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-implementation EXPORT ${PROJECT_NAME}-targets
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
	LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
	INCLUDES      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(EXPORT ${PROJECT_NAME}-targets
	DESTINATION ${INSTALL_CONFIGDIR}
	NAMESPACE Malloc::
	FILE ${PROJECT_NAME}-targets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	PATH_VARS INSTALL_CONFIGDIR
 	INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
	INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
 	VERSION ${PROJECT_VERSION}
 	COMPATIBILITY AnyNewerVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
 	DESTINATION ${INSTALL_CONFIGDIR}
)

enable_testing()
add_subdirectory(test)

add_custom_target(lint-fix
	COMMAND clang-format
	--style=file
	-i
	`echo "$<JOIN:$<TARGET_PROPERTY:${PROJECT_NAME},SOURCES>, >" `
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
