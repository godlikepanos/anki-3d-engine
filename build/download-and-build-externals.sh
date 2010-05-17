#! /bin/bash

cd ../../
svn checkout http://bullet.googlecode.com/svn/trunk/ bullet_svn
cd bullet_svn
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
make
cd ..
hg clone http://hg.libsdl.org/SDL SDL-hg
cd SDL-hg
./autogen.sh
./configure
make
cd ..
svn co https://glew.svn.sourceforge.net/svnroot/glew/trunk/glew glew
cd glew
make extensions
make
