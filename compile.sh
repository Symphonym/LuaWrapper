#!/bin/bash

PROJ_GENERATOR="Unix Makefiles"
PROJ_BUILD_TYPE="Release"

PROJ_BUILD_INDIRECTION_LVL="./../../"
PROJ_BUILD_RELEASE_DIR="./CMakeFiles/release/"
PROJ_BUILD_DEBUG_DIR="./CMakeFiles/debug/"

# Move to the correct build directory
if [[ ${PROJ_BUILD_TYPE} ==  "Release" ]]; then
	cd ${PROJ_BUILD_RELEASE_DIR}
elif [[ ${PROJ_BUILD_TYPE} == "Debug" ]]; then
	cd ${PROJ_BUILD_DEBUG_DIR}
fi

# Generate makefiles and compile program
cmake -G "${PROJ_GENERATOR}" -D CMAKE_BUILD_TYPE="${PROJ_BUILD_TYPE}" "${PROJ_BUILD_INDIRECTION_LVL}" && make all