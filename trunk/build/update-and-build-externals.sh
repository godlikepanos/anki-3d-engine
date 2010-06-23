#! /bin/bash

set -x
cd ../../bullet
svn update
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
make
cd ../SDL
hg pull
./autogen.sh
./configure
make
cd ../glew
svn update
cd auto
make destroy
cd ..
make extensions
make

