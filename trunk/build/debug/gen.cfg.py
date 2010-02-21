source_paths = [ "../../src/math/", "../../src/tokenizer/", "../../src/uncategorized/", "../../src/", "../../src/renderer/", "../../src/scene/", "../../src/ui/", "../../src/resources/", "../../src/utility/", "../../src/controllers/" ]

include_paths = list(source_paths)
#include_paths.extend( ["../../../bullet_svn/src/BulletDynamics", "../../../bullet_svn/src/BulletDynamics/ibmsdk", "../../../bullet_svn/src/BulletDynamics/Character", "../../../bullet_svn/src/BulletDynamics/Vehicle", "../../../bullet_svn/src/BulletDynamics/Dynamics", "../../../bullet_svn/src/BulletDynamics/ConstraintSolver", "../../../bullet_svn/src/ibmsdk", "../../../bullet_svn/src/BulletCollision", "../../../bullet_svn/src/BulletCollision/ibmsdk", "../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision", "../../../bullet_svn/src/BulletCollision/Gimpact", "../../../bullet_svn/src/BulletCollision/BroadphaseCollision", "../../../bullet_svn/src/BulletCollision/CollisionShapes", "../../../bullet_svn/src/BulletCollision/CollisionDispatch", "../../../bullet_svn/src/BulletMultiThreaded", "../../../bullet_svn/src/BulletMultiThreaded/SpuSampleTask", "../../../bullet_svn/src/BulletMultiThreaded/MiniCLTask", "../../../bullet_svn/src/BulletMultiThreaded/out", "../../../bullet_svn/src/BulletMultiThreaded/vectormath", "../../../bullet_svn/src/BulletMultiThreaded/vectormath/scalar", "../../../bullet_svn/src/BulletMultiThreaded/vectormath/scalar/cpp", "../../../bullet_svn/src/BulletMultiThreaded/SpuNarrowPhaseCollisionTask", "../../../bullet_svn/src/LinearMath", "../../../bullet_svn/src/LinearMath/ibmsdk", "../../../bullet_svn/src/BulletSoftBody", "../../../bullet_svn/src/MiniCL"] )
include_paths.extend( [ "../../../bullet_svn/src/" ] )

precompiled_headers = []
executable_name = "AnKi.bin"
compiler = "g++"
common_flags = ""
compiler_flags = "-c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_"
precompiled_headers_flags = "-x c++-header"
#linker_flags = "-L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -L/usr/lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib,-Bstatic -lSDL -lSDL_image -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -Wl,-Bdynamic -lGL -lGLU -ljpeg -lGLEW"
linker_flags = "-L/usr/lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib,-Bstatic -lSDL -lSDL_image -Wl,-Bdynamic -lGL -lGLU -ljpeg -lGLEW"
#linker_flags = "-rdynamic -static-libgcc -static-stdc++ -Bdynamic -lGL -lGLU -ljpeg -lGLEW -Bstatic `sdl-config --static-libs` -lSDL_image"
