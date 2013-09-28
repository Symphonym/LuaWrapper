# PROJ_BIN_DIR = Binary output directory
# PROJ_LIBRARY_DIR = Local library directory
# PROJ_INCLUDE_DIR = Local include directory
# PROJ_SYS_ARCH = System architechture, "x32" or "x64"


# Executable name
# Append "_dev" to exe name if debugging

set(PROJ_EXE_NAME luatest)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PROJ_EXE_NAME "${PROJ_EXE_NAME}_dev")
endif()

# Find libraries

find_library(LUA_LIB lua ${PROJ_LIBRARY_DIR})
find_library(DL_LIB dl ${PROJ_LIBRARY_DIR})

# Glob files, add exe and finally link libraries

file(GLOB PROJ_SOURCE_FILES "*.cpp")

if(PROJ_SOURCE_FILES)
	add_executable(${PROJ_EXE_NAME} ${PROJ_SOURCE_FILES})

	target_link_libraries(${PROJ_EXE_NAME} ${LUA_LIB})
	target_link_libraries(${PROJ_EXE_NAME} ${DL_LIB})
endif()