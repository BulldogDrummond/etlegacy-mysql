#!/bin/bash

_PREFIX="/opt/wolfet"

_SRC=`pwd`
BUILDDIR="${_SRC}/build"

HACK_CSTDIO=1

echo
echo "ET Legacy Server Builder"
echo "==============================="
echo

# cmake varialbes
RELEASE_TYPE="Release"
BUILD_CLIENT=0
BUNDLED_SDL=0
BUNDLED_JPEG=0
BUNDLED_LUA=1
BUNDLED_CURL=1
FEATURE_OGG_VORBIS=0
FEATURE_TRACKBASE=1
FEATURE_OMNIBOT=1
FEATURE_ANTICHEAT=1
FEATURE_MYSQL=1

mkdir -p ${BUILDDIR}
CLEANLIBS=0
if [[ ! -e "${_SRC}/libs/CMakeLists.txt" && ${CLEANLIBS} ]]; then
	echo "Getting bundled libs..."
	git submodule init
	git submodule update
fi

cd ${BUILDDIR}
echo "Configuring ET Legacy..."
_CFGSTRING="
        -DCMAKE_INSTALL_PREFIX=${_PREFIX}
	-DCMAKE_BUILD_TYPE=${RELEASE_TYPE}
	-DBUILD_CLIENT=${BUILD_CLIENT}
	-DBUILD_SERVER=1
	-DBUILD_MOD=1
	-DBUILD_MOD_PK3=0
	-DBUILD_PAK3_PK3=1
	-DBUNDLED_SDL=${BUNDLED_SDL}
	-DBUNDLED_JPEG=${BUNDLED_JPEG}
	-DBUNDLED_LUA=${BUNDLED_LUA}
	-DBUNDLED_CURL=${BUNDLED_CURL}
	-DCROSS_COMPILE32=1
	-DFEATURE_CURL=1
	-DFEATURE_OGG_VORBIS=${FEATURE_OGG_VORBIS}
	-DFEATURE_FREETYPE=0
	-DFEATURE_OPENAL=0
	-DFEATURE_TRACKBASE=${FEATURE_TRACKBASE}
	-DFEATURE_OMNIBOT=${FEATURE_OMNIBOT}
	-DFEATURE_ANTICHEAT=${FEATURE_ANTICHEAT}
	-DFEATURE_MYSQL=${FEATURE_MYSQL}
	-DBUILD_WITH_CSTDIO_INCLUDED=${HACK_CSTDIO}
"

cmake ${_CFGSTRING} ..

echo "Compiling ET Legacy..."
make ${MAKEOPTS}


