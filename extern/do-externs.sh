#! /bin/bash

set -x

EXTERN_DIR=$PWD

cd ../../bullet
svn update
#rm CMakeCache.txt
cmake -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_EXTRAS=OFF -DBUILD_DEMOS=OFF -DCMAKE_INSTALL_PREFIX=$PWD/install -DEXECUTABLE_OUTPUT_PATH=$PWD/install -DINCLUDE_INSTALL_DIR=$PWD/install -DLIBRARY_OUTPUT_PATH=$PWD/install -DLIB_DESTINATION=$PWD/install -DPKGCONFIG_INSTALL_PREFIX=$PWD/install
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`
make install
cd $EXTERN_DIR
rsync -avuzb --exclude .svn ../../bullet/install/include/* include/bullet
rsync -avuzb --exclude .svn ../../bullet/install/lib/* lib-x86-64-linux

cd ../../SDL
hg pull
./autogen.sh
./configure --prefix=$PWD/install
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`
cd $EXTERN_DIR
rsync -avuzb --exclude .hg ../../SDL/install/include/* include
rsync -avuzb --exclude .hg ../../SDL/install/lib/* lib-x86-64-linux


cd ../../SDL_image
hg pull
./autogen.sh
./configure --prefix=$PWD/install
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`
cd $EXTERN_DIR
rsync -avuzb --exclude .hg ../../SDL_image/install/include/* include
rsync -avuzb --exclude .hg ../../SDL_image/install/lib/* lib-x86-64-linux

cd ../../glew
svn update
cd auto
make destroy
cd ..
make extensions
make -j `cat /proc/cpuinfo | grep processor | wc -l`
make GLEW_DEST=$PWD/install install
cd $EXTERN_DIR
rsync -avuzb --exclude .svn ../../glew/install/include/* include
rsync -avuzb --exclude .svn ../../glew/install/lib64/* lib-x86-64-linux
