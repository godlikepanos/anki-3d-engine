sourcePaths = ["../../src/Math/", "../../src/Util/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Scene/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/", "../../src/Resources/Helpers/", "../../src/Resources/Core/"]

includePaths = []
includePaths.append("./")
includePaths.extend(list(sourcePaths))
includePaths.extend(["../../../bullet/src/", "../../../SDL/include", "../../../glew/include", "../../../SDL_image"])

precompiledHeaders = []

executableName = "anki"

compiler = "g++"

defines__ = "-DPLATFORM_LINUX -DREVISION=\\\"`svnversion -c ../..`\\\""

precompiledHeadersFlags = defines__ + " -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -s -msse3 -O3 -mtune=core2 -ffast-math"

compilerFlags = precompiledHeadersFlags + " -fsingle-precision-constant"

linkerFlags = "-rdynamic -L../../../SDL/build/.libs -L../../../SDL_image/.libs -L../../../glew/lib -L../../../bullet/src/BulletSoftBody -L../../../bullet/src/BulletDynamics -L../../../bullet/src/BulletCollision -L../../../bullet/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -lSDL -lboost_system -lboost_filesystem -Wl,-Bdynamic -lGL -ljpeg -lpng -ltiff"
