cmake_minimum_required(VERSION 2.8)
project(OPENGL_TEST_1)


set(PROJ_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(PROJ_LINUX_DIR "${CMAKE_SOURCE_DIR}/linux")
set(PROJ_WINDOWS_DIR "${CMAKE_SOURCE_DIR}/windows")
set(PROJ_MAC_DIR "${CMAKE_SOURCE_DIR}/osx")

set(EXECUTABLE_OUTPUT_PATH ${PROJ_BIN_DIR})

# Detect system architechture

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(PROJ_SYS_ARCH "x64")
else()
	set(PROJ_SYS_ARCH "x32")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions("-std=c++11")
	add_definitions("-Wall")
	#add_definitions("-Werror")
endif()

# Platform specific options

if(UNIX)
	set(PROJ_LIBRARY_DIR "${PROJ_LINUX_DIR}/${PROJ_SYS_ARCH}/lib")
	set(PROJ_INCLUDE_DIR "${PROJ_LINUX_DIR}/${PROJ_SYS_ARCH}/include")

	set(CMAKE_INSTALL_RPATH "\$ORIGIN/lib")
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 

	file(COPY ${PROJ_LIBRARY_DIR} DESTINATION ${PROJ_BIN_DIR})
elseif(WIN32)
	set(PROJ_LIBRARY_DIR "${PROJ_WINDOWS_DIR}/${PROJ_SYS_ARCH}/lib")
	set(PROJ_INCLUDE_DIR "${PROJ_WINDOWS_DIR}/${PROJ_SYS_ARCH}/include")

	file(COPY ${PROJ_LIBRARY_DIR}/ DESTINATION ${PROJ_BIN_DIR})
elseif(APPLE)
	set(PROJ_LIBRARY_DIR "${PROJ_APPLE_DIR}/${PROJ_SYS_ARCH}/lib")
	set(PROJ_INCLUDE_DIR "${PROJ_APPLE_DIR}/${PROJ_SYS_ARCH}/include")
endif()

# Include headers

include_directories(${PROJ_INCLUDE_DIR})

add_subdirectory(src)

