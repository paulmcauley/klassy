#!/bin/sh
#
# Modified from https://github.com/kupiqu/SierraBreezeEnhanced
# - Recreate build
# - Build
# - Install

ORIGINAL_DIR=$(pwd)

rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make -j$(nproc)
sudo make install

cd $ORIGINAL_DIR
