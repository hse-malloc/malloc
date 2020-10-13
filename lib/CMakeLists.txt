cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(libmalloc
	VERSION 0.1
	DESCRIPTION "custom malloc() implementation with randomized addresses"
	LANGUAGES CXX
)

add_library(malloc STATIC
	src/malloc.cpp
)
set_target_properties(malloc PROPERTIES
	EXPORT_NAME Malloc
	PUBLIC_HEADER
		include/malloc/malloc.h
)
target_compile_features(malloc PRIVATE cxx_std_20)
target_compile_options(malloc PRIVATE -Wall -Wextra -pedantic)

target_include_directories(malloc
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/malloc>
		$<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
install(TARGETS malloc EXPORT malloc-targets
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/malloc)

install(EXPORT malloc-targets
	NAMESPACE Malloc::
	DESTINATION ${INSTALL_CONFIGDIR}
	FILE malloc-targets.cmake
)

set(INCLUDE_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/malloc-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/malloc-config.cmake
	INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
	PATH_VARS INCLUDE_INSTALL_DIR
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/malloc-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/malloc-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/malloc-config-version.cmake
	DESTINATION ${INSTALL_CONFIGDIR}
)

enable_testing()
add_subdirectory(test)
