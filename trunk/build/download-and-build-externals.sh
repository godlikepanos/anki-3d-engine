#! /bin/bash

set -x

cd ../../
svn checkout http://bullet.googlecode.com/svn/trunk/ bullet
cd bullet
cmake . -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`

cd ..
hg clone http://hg.libsdl.org/SDL SDL
cd SDL
./autogen.sh
./configure
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`

cd ..
svn co https://glew.svn.sourceforge.net/svnroot/glew/trunk/glew glew
cd glew
make extensions
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`

cd ..
hg clone http://hg.libsdl.org/SDL_image SDL_image
cd SDL_image
./autogen.sh
./configure
nice make -j `cat /proc/cpuinfo | grep processor | wc -l`

