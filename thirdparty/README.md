This repository contains all the external libraries for AnKi.

When copying the don't copy the 2 .c files that contain the main(). You don't
need those.

Change the LUAI_USER_ALIGNMENT_T define to contain a high enough alignment.

For zlib and libpng just download the source from freeimage library.

For assimp buld it first to generate some files, copy code, contrib, include. See the cmake file and do the boost workaround. Remove the cppunit* dir from contrib.
