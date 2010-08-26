#! /bin/bash

set -x
cd ../../
svn checkout http://bullet.googlecode.com/svn/trunk/ bullet
hg clone http://hg.libsdl.org/SDL SDL
svn checkout https://glew.svn.sourceforge.net/svnroot/glew/trunk/glew glew
