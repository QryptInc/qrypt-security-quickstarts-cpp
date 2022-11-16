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
    echo "--build_encrypt_tool      Build Encryption Tool. This function requires the Openssl library pre-installed in the system."
    echo ""    
    echo "--build_type=<option>     Defaults to Release."
    echo "                          Release - Build targeted for releasing."
    echo "                          Debug   - Build targeted for debugging."
    echo ""
    echo "--cross_build=<platform>  Defaults to none."
    echo "                          Android - Build for an Android host system."
    echo ""
    echo "--cross_build_abi=<arch>  Required when cross-building."
    echo "                          arm64-v8a - Build for arm v8 Android hardware."
    echo "                          x86_64    - Build for 64-bit Android emulators."
    echo ""

    exit
}

BUILD_ENCRYPT_TOOL=OFF

# Parse input arguments
for i in "$@"
do
case $i in
    --help)
    usage
    shift
    ;;
    --build_encrypt_tool)
    BUILD_ENCRYPT_TOOL=ON
    shift
    ;;    
    --build_type=*)
    BUILD_TYPE="${i#*=}"
    shift
    ;;
    --cross_build=*)
    SYSTEM_NAME="${i#*=}"
    shift
    ;;
    --cross_build_abi=*)
    HOST_ABI="${i#*=}"
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
if [[ "$SYSTEM_NAME" != "" && "$SYSTEM_NAME" != "Android" ]]; then
    echo "Invalid --cross_build target: $SYSTEM_NAME"
    usage
fi
if [[ "$SYSTEM_NAME" == "Android" ]]; then
    if [[ "$HOST_ABI" != "arm64-v8a" && "$HOST_ABI" != "x86_64" ]]; then
        echo "Invalid --cross_build_abi host: $HOST_ABI"
        usage
    fi
    echo "***************************************"
    echo "* ANDROID CROSS-BUILD SETUP"
    echo "***************************************"
    if [[ "$ANDROID_HOME" == "" || ! -d "$ANDROID_HOME" ]]; then
        echo "\$ANDROID_HOME is invalid or not set!"
        echo "    Expecting a path to the Android SDK"
    fi
    if [[ "$ANDROID_NDK_HOME" == "" || ! -d "$ANDROID_NDK_HOME" ]]; then
        echo "\$ANDROID_NDK_HOME is invalid or not set!"
        echo "    Expecting a path to the Android NDK (ie '\$ANDROID_HOME/ndk/21.3.6528147/')"
    fi
    if [[ "$ANDROID_CMAKE_HOME" == "" || ! -d "$ANDROID_CMAKE_HOME" ]]; then
        echo "\$ANDROID_CMAKE_HOME is invalid or not set!"
        echo "    Expecting a path to the Android cmake (ie '\$ANDROID_HOME/cmake/3.10.2.4988404/')"
    fi
    if [[ "$ANDROID_HOME" == "" || ! -d "$ANDROID_HOME" || 
          "$ANDROID_NDK_HOME" == "" || ! -d "$ANDROID_NDK_HOME" || 
          "$ANDROID_CMAKE_HOME" == "" || ! -d "$ANDROID_CMAKE_HOME" ]]; then
        exit
    fi
    echo "HOST_ABI=$HOST_ABI"
    echo "ANDROID_HOME=$ANDROID_HOME"
    echo "ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
    echo "ANDROID_CMAKE_HOME=$ANDROID_CMAKE_HOME"
    VERSION=25
    CMAKE_ARGS="\
        -G Ninja \
        -DANDROID_ABI=$HOST_ABI \
        -DANDROID_NDK=$ANDROID_NDK_HOME \
        -DANDROID_PLATFORM=android-$VERSION \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
        -DCMAKE_ANDROID_ARCH_ABI=$HOST_ABI \
        -DCMAKE_ANDROID_NDK=$ANDROID_NDK_HOME \
        -DCMAKE_MAKE_PROGRAM=$ANDROID_CMAKE_HOME/bin/ninja \
        -DCMAKE_SYSTEM_NAME=Android \
        -DCMAKE_SYSTEM_VERSION=$VERSION"
else
    CMAKE_ARGS=""
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
cmake -DBUILD_ENCRYPT_TOOL=$BUILD_ENCRYPT_TOOL -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_ARGS ..
cmake --build .

cd ..
