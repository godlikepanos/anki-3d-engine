sourcePaths = ["../../src"]
sourcePaths.extend(list(walkDir("../../src")))

includePaths = ["./"]
includePaths.extend(list(sourcePaths))
includePaths.extend(["../../extern/include", "../../extern/include/bullet", "/usr/include/python2.6"])

executableName = "anki"

compiler = "g++-4.5"

compilerFlags = "-DPLATFORM_LINUX -DMATH_INTEL_SIMD -DNDEBUG -DBOOST_DISABLE_ASSERTS -DREVISION=\\\"`svnversion ../..`\\\" -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -Wno-long-long -pipe -fsingle-precision-constant -msse4 -O3 -mtune=core2 -ffast-math -flto"

linkerFlags = "-rdynamic -flto -L../../extern/lib-x86-64-linux -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lGLU -Wl,-Bdynamic -lGL -ljpeg -lSDL -lpng -lpython2.6 -lboost_system -lboost_python -lboost_filesystem -lboost_thread"
