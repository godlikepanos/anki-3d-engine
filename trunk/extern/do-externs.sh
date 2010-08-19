#! /bin/bash

#set -x

cd ../../bullet
cmake . -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$SOURCE_DIR/install -DEXECUTABLE_OUTPUT_PATH=$SOURCE_DIR/bullet/install -DINCLUDE_INSTALL_DIR=$SOURCE_DIR/bullet/install -DLIBRARY_OUTPUT_PATH=$SOURCE_DIR/bullet/install -DLIB_DESTINATION=$SOURCE_DIR/bullet/install -DPKGCONFIG_INSTALL_PREFIX=$SOURCE_DIR/bullet/install
make clean
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`
make install
cd -
rsync -avuzb ../../bullet/install/include include/bullet
rsync -avuzb ../../bullet/install/lib include/bullet/lib-x86-64-linux

