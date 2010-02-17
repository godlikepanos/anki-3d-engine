include_paths = [ "../../src/math/", "../../src/tokenizer/", "../../src/uncategorized/", "../../src/", "../../src/renderer/", "../../src/scene/", "../../src/resources/", "../../src/utility/", "../../src/ui/", "../../src/controllers/" ]
executable_name = "AnKi.bin"
precompiled_headers = []
compiler = "gcc"
common_flags = "-fopenmp"
compiler_flags = "-c -pedantic-errors -pedantic -ansi -Wall -Wextra -pipe `sdl-config --cflags` -s -msse3 -O3 -mtune=core2 -fsingle-precision-constant -ffast-math -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_"
precompiled_headers_flags = ""
linker_flags = "-lGL -lGLU -lGLEW -lSDL_image `sdl-config --static-libs` -ljpeg -lgomp -rdynamic"
