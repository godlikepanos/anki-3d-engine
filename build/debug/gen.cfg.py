sourcePaths = ["../../src/Math/", "../../src/Util/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Scene/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/", "../../src/Resources/Helpers/"]

includePaths = []
includePaths.append("./")
includePaths.extend(list(sourcePaths))
includePaths.extend([ "../../../bullet/src/", "../../../SDL/include", "../../../glew/include" ])

#precompiledHeaders = ["../../src/Util/Common.h", "/usr/include/boost/filesystem.hpp", "/usr/include/boost/ptr_container/ptr_vector.hpp"]
precompiledHeaders = []

executableName = "anki"

compiler = "g++"

defines__ = "-DDEBUG_ENABLED -DPLATFORM_LINUX -DREVISION=\\\"`svnversion -c ../..`\\\""

precompiledHeadersFlags = defines__ + " -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg"

compilerFlags = precompiledHeadersFlags + " -fsingle-precision-constant"

linkerFlags = "-rdynamic -L../../../SDL/build/.libs -L../../../glew/lib -L../../../bullet/src/BulletSoftBody -L../../../bullet/src/BulletDynamics -L../../../bullet/src/BulletCollision -L../../../bullet/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -lSDL -lboost_system -lboost_filesystem -Wl,-Bdynamic -lGL -ljpeg -lpng -ltiff -pg"
