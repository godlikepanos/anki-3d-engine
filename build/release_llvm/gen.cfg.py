include_paths = [ "../../src/math/", "../../src/tokenizer/", "../../src/uncategorized/", "../../src/", "../../src/renderer/", "../../src/scene/", "../../src/resources/", "../../src/utility/", "../../src/ui/" ]
precompiled_headers = []
project_name = "AnKi"
compiler = "/usr/lib/llvm/llvm/gcc-4.2/bin/llvm-g++"
common_flags = "-emit-llvm"
compiler_flags = "-c `sdl-config --cflags` -O3 -D_TERMINAL_COLORING_ -D_PLATFORM_LINUX_"
precompiled_headers_flags = ""
linker_flags = "-lGL -lGLU -lGLEW -lSDL_image `sdl-config --static-libs` -ljpeg -lgomp"
