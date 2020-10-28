cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(malloc
	VERSION 1.0
	DESCRIPTION "custom malloc() implementation with randomized addresses"
	LANGUAGES CXX
)

add_library(malloc SHARED
	src/malloc.cpp
	src/system/system.cpp
	src/memory/memory_control_block.cpp
	src/memory/allocator.cpp
)
add_library(Malloc::Malloc ALIAS malloc)
set_target_properties(malloc PROPERTIES
	EXPORT_NAME Malloc
	PUBLIC_HEADER
		include/malloc/malloc.h
)

target_include_directories(malloc
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/malloc>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(malloc PRIVATE cxx_std_20)
target_compile_options(malloc PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
    -Wall -Wextra -Wpedantic>)

include(GNUInstallDirs)
install(TARGETS malloc EXPORT malloc-targets
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/malloc
	LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
	INCLUDES      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/malloc
)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/malloc)

install(EXPORT malloc-targets
	DESTINATION ${INSTALL_CONFIGDIR}
	NAMESPACE Malloc::
	FILE malloc-targets.cmake
)

include(CMakePackageConfigHelpers)

#set(INCLUDE_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/config.cmake.in
 	${CMAKE_CURRENT_BINARY_DIR}/malloc-config.cmake
	PATH_VARS INSTALL_CONFIGDIR
 	INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
	INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/malloc-config-version.cmake
 	VERSION ${PROJECT_VERSION}
 	COMPATIBILITY AnyNewerVersion
)

install(FILES
 	${CMAKE_CURRENT_BINARY_DIR}/malloc-config.cmake
 	${CMAKE_CURRENT_BINARY_DIR}/malloc-config-version.cmake
 	DESTINATION ${INSTALL_CONFIGDIR}
)

enable_testing()
add_subdirectory(test)
