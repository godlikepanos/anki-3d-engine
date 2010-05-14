sourcePaths = [ "../../src/Math/", "../../src/Util/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Scene/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/", "../../src/Renderer2/" ]

includePaths = list(sourcePaths)
includePaths.extend( [ "../../../bullet_svn/src/", "../../../SDL-hg/include" ] ) # the extra libs paths

precompiledHeaders = []
executableName = "AnKi.bin"
compiler = "g++"
commonFlags = ""
compilerFlags = "-c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg -fsingle-precision-constant -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_ -DPLATFORM=LINUX -DREVISION=\\\"`svnversion -c ../..`\\\" "
precompiledHeadersFlags = "-x c++-header"
linkerFlags = "-rdynamic -L../../../SDL-hg/build/.libs -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -lSDL -Wl,-Bdynamic -lGL -ljpeg -lpng -ltiff" # a few libs are now static
