sourcePaths = [ "../../src/Math/", "../../src/Util/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Scene/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/", "../../src/Resources/Helpers/" ]

includePaths = list(sourcePaths)
includePaths.extend( [ "../../../bullet_svn/src/", "../../../SDL-hg/include", "../../glew/include" ] )

precompiledHeaders = []
executableName = "AnKi.bin"
compiler = "g++"
commonFlags_ = ""
compilerFlags = "-c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg -fsingle-precision-constant -DDEBUG_ENABLED -DPLATFORM_LINUX -DREVISION=\\\"`svnversion -c ../..`\\\" "
precompiledHeadersFlags = compilerFlags + " -x "
linkerFlags = "-rdynamic -L../../../SDL-hg/build/.libs -L../../../glew/lib -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -lSDL -Wl,-Bdynamic -lGL -ljpeg -lpng -ltiff"
