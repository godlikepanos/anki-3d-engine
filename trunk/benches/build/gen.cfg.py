sourcePaths = walkDir("../../src")
sourcePaths.extend(list(walkDir("../", True)))

includePaths = ["./"]
includePaths.extend(list(sourcePaths))
includePaths.extend(["../../extern/include", "../../extern/include/bullet", "/usr/include/python2.6"])

executableName = "anki-benches"

compiler = "g++"

compilerFlags = "-DDEBUG_ENABLED=0 -DBOOST_DISABLE_ASSERTS -DNDEBUG -DPLATFORM_LINUX -DMATH_INTEL_SIMD -DREVISION=\\\"`svnversion -c ../..`\\\" -c -msse4 -pedantic-errors -pedantic -ansi -Wall -Wextra -W -Wno-long-long -pipe -s -O3 -mtune=core2 -ffast-math -fsingle-precision-constant"

linkerFlags = "-rdynamic -pg -L../../extern/lib-x86-64-linux -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lGLU -Wl,-Bdynamic -lGL -ljpeg -lSDL -lpng -lpython2.6 -lboost_system -lboost_python -lboost_filesystem -lboost_thread"
