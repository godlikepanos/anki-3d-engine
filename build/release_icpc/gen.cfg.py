include_paths = [ "../../src/math/", "../../src/renderer/", "../../src/tokenizer/", "../../src/", "../../src/uncategorized/" ]
precompiled_headers = []
project_name = "AnKi"
compiler = "/home/godlike/.intel/Compiler/11.1/046/bin/intel64/icpc"
common_flags = "-ipo -parallel -openmp"
compiler_flags = "-c `sdl-config --cflags` -O3 -ipo -ip -D_TERMINAL_COLORING_ -D_PLATFORM_LINUX_"
precompiled_headers_flags = ""
linker_flags = "-lGL -lGLU -lGLEW -lSDL_image `sdl-config --static-libs` -ljpeg -lgomp"
