sourcePaths = ["../../src/Math/", "../../src/Util/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Scene/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/", "../../src/Resources/Helpers/", "../../src/Resources/Core/"]

includePaths = []
includePaths.append("./")
includePaths.extend(list(sourcePaths))
includePaths.extend(["../../extern/include", "../../extern/include/bullet"])

#precompiledHeaders = ["../../src/Util/Common.h", "/usr/include/boost/filesystem.hpp", "/usr/include/boost/ptr_container/ptr_vector.hpp"]
precompiledHeaders = []

executableName = "anki"

compiler = "g++"

defines__ = "-DPLATFORM_LINUX -DREVISION=\\\"`svnversion -c ../..`\\\""

precompiledHeadersFlags = defines__ + " -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -s -msse3 -O3 -mtune=core2 -ffast-math"

compilerFlags = precompiledHeadersFlags + " -fsingle-precision-constant"

linkerFlags = "-rdynamic -L../../extern/lib-x86-64-linux -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lGLU -lboost_system -lboost_filesystem -Wl,-Bdynamic -lSDL -lGL -ljpeg -lpng -ltiff"
