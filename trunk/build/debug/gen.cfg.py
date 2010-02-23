source_paths = [ "../../src/math/", "../../src/tokenizer/", "../../src/uncategorized/", "../../src/", "../../src/renderer/", "../../src/scene/", "../../src/ui/", "../../src/resources/", "../../src/utility/", "../../src/controllers/" ]

include_paths = list(source_paths)
include_paths.extend( [ "../../../bullet_svn/src/" ] ) # the bullet svn path

precompiled_headers = []
executable_name = "AnKi.bin"
compiler = "g++"
common_flags = ""
compiler_flags = "-c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_"
precompiled_headers_flags = "-x c++-header"
#linker_flags = "-rdynamic -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -Wl,-Bdynamic -lSDL -lpthread -lGL -ljpeg -lpng -ltiff -lesd -lcaca -laa -ldirectfb -laudio -lpulse-simple" # a few libs are now static
linker_flags = "-rdynamic -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -Wl,-Bdynamic -lSDL -lGL -ljpeg -lpng -ltiff" # a few libs are now static
