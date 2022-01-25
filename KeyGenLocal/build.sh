#!/bin/bash

# Copyright © 2018-2022, Qrypt, Inc. All rights reserved.

# Display help menu
usage() {
    echo ""
    echo "build.sh"
    echo "========"
    echo ""
    echo "General build script used to build the project."
    echo ""
    echo "Options:"
    echo "--help                    Displays help menu"
    echo ""
    echo "--build_type=<option>     Defaults to Release."
    echo "                          Release - Build targeted for releasing."
    echo "                          Debug   - Build targeted for debugging."
    echo ""
    echo ""

    exit
}

# Parse input arguments
for i in "$@"
do
case $i in
    --help)
    usage
    shift
    ;;
    --build_type=*)
    BUILD_TYPE="${i#*=}"
    shift
    ;;
    *)
    echo "Unknown option: $i"
    usage
    shift
    ;;
esac
done

# Validate input arguments and set defaults
if [[ "$BUILD_TYPE" == "" ]]; then
    BUILD_TYPE="Release"
fi
if [[ "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "Debug" ]]; then
    echo "Invalid --build_type: $BUILD_TYPE"
    usage
fi

echo "***************************************"
echo "* CLEAN"
echo "***************************************"
rm -rvf build
mkdir build
cd build

echo "***************************************"
echo "* BUILD"
echo "***************************************"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
cmake --build .

cd ..
