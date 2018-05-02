#!/usr/bin/env bash

set -x
set -e

BUILD_DIR=`pwd`/.test
PROJECT_DIR=`pwd`

if [[ ! -d ${BUILD_DIR} ]]
then
    mkdir ${BUILD_DIR}
    cd ${BUILD_DIR}

    # clone rack
    git clone --recurse-submodules git@github.com:VCVRack/Rack.git
    cd Rack
    git checkout v0.5.1
    git submodule update --init --recursive

    # apply xmonad patch
    URL="https://github.com/sorki/Rack/commit/e1a81a44e400c23b5239d941e9cc4943009ea714.patch"
    curl $URL | git apply -v --index
    git commit -m "Applied $URL."

    # install needed plugins
    cd plugins
    git clone git@github.com:VCVRack/Fundamental.git
    cd Fundamental
    git checkout v0.5.1

fi

cd ${BUILD_DIR}/Rack


# copy code
TARGET=plugins/vcvrecorder
rm -rf ${TARGET}
mkdir -p ${TARGET}
cp -r \
   ${PROJECT_DIR}/{src,res,Makefile,portaudio} \
   ${TARGET}

# compile
FLAGS=$( pkg-config --cflags-only-I glew glfw3 jansson samplerate libcurl libzip rtmidi rtaudio gtk+-2.0 )
export FLAGS
make allplugins
make VERSION=0.5.1

# run
./Rack
