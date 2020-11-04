cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(malloc
	VERSION 1.0
	DESCRIPTION "custom malloc() implementation with randomized addresses"
	LANGUAGES CXX
)

add_library(${PROJECT_NAME}
	src/malloc.cpp
	src/system/system.cpp
	src/memory/memory_control_block.cpp
	src/memory/allocator.cpp
)
add_library(Malloc::Malloc ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	EXPORT_NAME Malloc
	PUBLIC_HEADER
		include/malloc.h
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
		-Wall
		-Wextra
		-Wpedantic
	>)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
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

#set(INCLUDE_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
