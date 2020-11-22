cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(malloc
	VERSION 1.0
	DESCRIPTION "custom malloc() implementation with randomized addresses"
	LANGUAGES CXX
)

add_library(hse_${PROJECT_NAME}
        src/malloc.cpp
        src/malloc.h
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
        src/concepts/numbers.h
)

set_target_properties(hse_${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    EXPORT_NAME HSE_Malloc
)

target_compile_definitions(hse_${PROJECT_NAME}
    PRIVATE "$<$<CONFIG:DEBUG>:HSE_MALLOC_DEBUG>"
    PRIVATE "$<$<PLATFORM_ID:Linux,Darwin>:HAVE_DEV_URANDOM>"
    PRIVATE "$<$<PLATFORM_ID:Linux,Darwin>:HAVE_MMAP>"
    PRIVATE "$<$<PLATFORM_ID:Windows>:HAVE_VIRTUAL_ALLOC>"
    PRIVATE "$<$<BOOL:HSE_MALLOC_NO_RANDOM>:HSE_MALLOC_NO_RANDOM>"
)
target_include_directories(hse_${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:src>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_features(hse_${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(hse_${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
                -Wall
                -Wextra
                -Wpedantic
    >
)

add_library(${PROJECT_NAME}
    include/malloc.h
    src/malloc_std.cpp
)

add_library(Malloc::Malloc ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} hse_${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME} PROPERTIES
	INCLUDE_DIRECTORIES $<TARGET_PROPERTY:hse_${PROJECT_NAME},INCLUDE_DIRECTORIES>
	COMPILE_FEATURES $<TARGET_PROPERTY:hse_${PROJECT_NAME},COMPILE_FEATURES>
	COMPILE_OPTIONS $<TARGET_PROPERTY:hse_${PROJECT_NAME},COMPILE_OPTIONS>
	LINK_OPTIONS $<TARGET_PROPERTY:hse_${PROJECT_NAME},LINK_OPTIONS>
	EXPORT_NAME Malloc
	PUBLIC_HEADER
		include/malloc.h
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} hse_${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
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

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    enable_testing()
    add_subdirectory(test)
endif()

add_custom_target(lint-fix
	COMMAND clang-format
	--style=file
	-i
	`echo "$<JOIN:$<TARGET_PROPERTY:${PROJECT_NAME},SOURCES>, >" `
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
