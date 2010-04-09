sourcePaths = [ "../../src/Math/", "../../src/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/" ]

include_paths = list(sourcePaths)
include_paths.extend( [ "../../../bullet_svn/src/" ] ) # the bullet svn path

precompiledHeaders = []
executableName = "AnKi.bin"
compiler = "g++"
commonFlags = ""
compilerFlags = "-c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -fsingle-precision-constant -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_"
precompiledHeadersFlags = "-x c++-header"
#linkerFlags = "-rdynamic -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -Wl,-Bdynamic -lSDL -lpthread -lGL -ljpeg -lpng -ltiff -lesd -lcaca -laa -ldirectfb -laudio -lpulse-simple" # a few libs are now static
linkerFlags = "-rdynamic -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -Wl,-Bdynamic -lSDL -lGL -ljpeg -lpng -ltiff" # a few libs are now static
