sourcePaths = ["../../src/Math/", "../../src/Util/Tokenizer/", "../../src/Misc/", "../../src/", "../../src/Renderer/", "../../src/Scene/", "../../src/Ui/", "../../src/Resources/", "../../src/Util/", "../../src/Scene/Controllers/", "../../src/Physics/", "../../src/Renderer/BufferObjects/", "../../src/Resources/Helpers/", "../../src/Resources/Core/", "../../src/Core/", "../../src/Scripting/", "../../src/Scripting/Math", "../../src/Scripting/Util", "../../src/Scripting/Core", "../../src/Scripting/Scene", "../../src/Scripting/Renderer"]

includePaths = []
includePaths.append("./")
includePaths.extend(list(sourcePaths))
includePaths.extend(["../../extern/include", "../../extern/include/bullet", "/usr/include/python2.6"])

executableName = "anki"

compiler = "g++"

compilerFlags = "-DDEBUG_ENABLED=1 -DPLATFORM_LINUX -DREVISION=\\\"`svnversion -c ../..`\\\" -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -Wno-long-long -pipe -O0 -g3 -pg -fsingle-precision-constant"

linkerFlags = "-rdynamic -pg -L../../extern/lib-x86-64-linux -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lGLU -Wl,-Bdynamic -lGL -ljpeg -lSDL -lpng -lpython2.6 -lboost_system -lboost_python -lboost_filesystem -lboost_thread"
