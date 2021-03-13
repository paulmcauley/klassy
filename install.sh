#!/bin/sh
#
# - Recreate build
# - Build
# - Install

ORIGINAL_DIR=$(pwd)

rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_LIBDIR=lib64 -DBUILD_TESTING=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make
sudo make install

cd $ORIGINAL_DIR
