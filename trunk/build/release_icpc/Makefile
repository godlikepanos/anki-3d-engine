CXX = /home/godlike/.intel/Compiler/11.1/046/bin/intel64/icpc
COMMONFLAGS = -ipo -parallel -openmp
CFLAGS = $(COMMONFLAGS) -c `sdl-config --cflags` -O3 -ipo -ip -D_TERMINAL_COLORING_ -D_PLATFORM_LINUX_
PHFLAGS = $(CFLAGS) 
LFLAGS = $(COMMONFLAGS) -lGL -lGLU -lGLEW -lSDL_image `sdl-config --static-libs` -ljpeg -lgomp
EXECUTABLE = AnKi
INCPATH = -I../../src/math/ -I../../src/renderer/ -I../../src/tokenizer/ -I../../src/ -I../../src/uncategorized/ 
SOURCES = ../../src/renderer/renderer.cpp ../../src/renderer/r_is_shadows.cpp ../../src/renderer/r_pps_bloom.cpp ../../src/renderer/r_is.cpp ../../src/renderer/r_ms_earlyz.cpp ../../src/renderer/r_pps.cpp ../../src/renderer/r_bs.cpp ../../src/renderer/r_dbg.cpp ../../src/renderer/r_ms.cpp ../../src/renderer/r_pps_lscatt.cpp ../../src/renderer/r_pps_ssao.cpp ../../src/tokenizer/scanner.cpp ../../src/main.cpp ../../src/uncategorized/material.cpp ../../src/uncategorized/mesh.cpp ../../src/uncategorized/input.cpp ../../src/uncategorized/engine_class.cpp ../../src/uncategorized/collision.cpp ../../src/uncategorized/handlers.cpp ../../src/uncategorized/skybox.cpp ../../src/uncategorized/camera.cpp ../../src/uncategorized/object.cpp ../../src/uncategorized/shaders.cpp ../../src/uncategorized/model.cpp ../../src/uncategorized/hud.cpp ../../src/uncategorized/map.cpp ../../src/uncategorized/texture.cpp ../../src/uncategorized/memory.cpp ../../src/uncategorized/common.cpp ../../src/uncategorized/smodel.cpp ../../src/uncategorized/resource.cpp ../../src/uncategorized/particles.cpp ../../src/uncategorized/scene.cpp ../../src/uncategorized/shader_parser.cpp ../../src/uncategorized/spatial.cpp ../../src/uncategorized/lights.cpp 
OBJECTS = renderer.o r_is_shadows.o r_pps_bloom.o r_is.o r_ms_earlyz.o r_pps.o r_bs.o r_dbg.o r_ms.o r_pps_lscatt.o r_pps_ssao.o scanner.o main.o material.o mesh.o input.o engine_class.o collision.o handlers.o skybox.o camera.o object.o shaders.o model.o hud.o map.o texture.o memory.o common.o smodel.o resource.o particles.o scene.o shader_parser.o spatial.o lights.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	@$(CXX) $(LFLAGS) $(OBJECTS) -o $(EXECUTABLE)
	@echo All Done!

renderer.o: ../../src/renderer/renderer.cpp /usr/include/stdlib.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdio.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/jpeglib.h /usr/include/jconfig.h /usr/include/jmorecfg.h \
 ../../src/renderer/renderer.h ../../src/uncategorized/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h
	@echo Compiling renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/renderer.cpp

r_is_shadows.o: ../../src/renderer/r_is_shadows.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h
	@echo Compiling r_is_shadows.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_is_shadows.cpp

r_pps_bloom.o: ../../src/renderer/r_pps_bloom.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h
	@echo Compiling r_pps_bloom.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps_bloom.cpp

r_is.o: ../../src/renderer/r_is.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/camera.h \
 /usr/include/c++/4.4.1/cstring /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/collision.h ../../src/uncategorized/object.h \
 ../../src/uncategorized/scene.h ../../src/uncategorized/spatial.h \
 ../../src/uncategorized/lights.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/material.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/smodel.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_is.cpp

r_ms_earlyz.o: ../../src/renderer/r_ms_earlyz.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h
	@echo Compiling r_ms_earlyz.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_ms_earlyz.cpp

r_pps.o: ../../src/renderer/r_pps.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h
	@echo Compiling r_pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps.cpp

r_bs.o: ../../src/renderer/r_bs.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/camera.h \
 /usr/include/c++/4.4.1/cstring /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/collision.h ../../src/uncategorized/object.h \
 ../../src/uncategorized/scene.h ../../src/uncategorized/spatial.h \
 ../../src/uncategorized/lights.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/material.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/smodel.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_bs.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_bs.cpp

r_dbg.o: ../../src/renderer/r_dbg.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/texture.h /usr/include/c++/4.4.1/limits \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h
	@echo Compiling r_dbg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_dbg.cpp

r_ms.o: ../../src/renderer/r_ms.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/camera.h \
 /usr/include/c++/4.4.1/cstring /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/collision.h ../../src/uncategorized/object.h \
 ../../src/uncategorized/scene.h ../../src/uncategorized/spatial.h \
 ../../src/uncategorized/lights.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/material.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/smodel.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_ms.cpp

r_pps_lscatt.o: ../../src/renderer/r_pps_lscatt.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h
	@echo Compiling r_pps_lscatt.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps_lscatt.cpp

r_pps_ssao.o: ../../src/renderer/r_pps_ssao.cpp ../../src/renderer/renderer.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/scene.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/lights.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h
	@echo Compiling r_pps_ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps_ssao.cpp

scanner.o: ../../src/tokenizer/scanner.cpp /usr/include/c++/4.4.1/fstream \
 /usr/include/c++/4.4.1/istream /usr/include/c++/4.4.1/ios \
 /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/stdio.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc /usr/include/c++/4.4.1/iomanip \
 ../../src/tokenizer/scanner.h ../../src/uncategorized/common.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/iostream /usr/include/c++/4.4.1/vector \
 /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc
	@echo Compiling scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/tokenizer/scanner.cpp

main.o: ../../src/main.cpp /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/ostream /usr/include/c++/4.4.1/ios \
 /usr/include/c++/4.4.1/iosfwd /usr/include/c++/4.4.1/bits/stringfwd.h \
 /usr/include/c++/4.4.1/bits/postypes.h /usr/include/c++/4.4.1/cwchar \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/bits/istream.tcc /usr/include/c++/4.4.1/fstream \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/typeinfo \
 ../../src/uncategorized/common.h /usr/include/stdlib.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/input.h /usr/include/SDL/SDL.h \
 /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wordsize.h \
 /usr/include/iconv.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_version.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h ../../src/uncategorized/handlers.h \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/renderer/renderer.h \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/uncategorized/resource.h ../../src/uncategorized/hud.h \
 ../../src/uncategorized/particles.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/material.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/lights.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/spatial.h ../../src/uncategorized/scene.h \
 ../../src/uncategorized/skybox.h ../../src/tokenizer/scanner.h \
 /usr/include/c++/4.4.1/sstream /usr/include/c++/4.4.1/bits/sstream.tcc \
 ../../src/uncategorized/map.h ../../src/uncategorized/model.h \
 ../../src/uncategorized/shader_parser.h
	@echo Compiling main.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/main.cpp

material.o: ../../src/uncategorized/material.cpp /usr/include/string.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 ../../src/uncategorized/material.h ../../src/uncategorized/common.h \
 /usr/include/stdio.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/sys/types.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/tokenizer/parser.h \
 ../../src/uncategorized/texture.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/renderer/renderer.h
	@echo Compiling material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/material.cpp

mesh.o: ../../src/uncategorized/mesh.cpp /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/ostream /usr/include/c++/4.4.1/ios \
 /usr/include/c++/4.4.1/iosfwd /usr/include/c++/4.4.1/bits/stringfwd.h \
 /usr/include/c++/4.4.1/bits/postypes.h /usr/include/c++/4.4.1/cwchar \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/stdio.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/bits/istream.tcc /usr/include/c++/4.4.1/fstream \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/common.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/material.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/object.h ../../src/renderer/vbo.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/renderer/renderer.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/tokenizer/parser.h
	@echo Compiling mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/mesh.cpp

input.o: ../../src/uncategorized/input.cpp ../../src/uncategorized/input.h \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /usr/include/sys/types.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/bits/wordsize.h \
 /usr/include/bits/sigset.h /usr/include/time.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/wordsize.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/xlocale.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wchar.h \
 /usr/include/bits/wordsize.h /usr/include/ctype.h /usr/include/iconv.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_version.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h ../../src/uncategorized/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/handlers.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/renderer/renderer.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h
	@echo Compiling input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/input.cpp

engine_class.o: ../../src/uncategorized/engine_class.cpp /usr/include/stdio.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc
	@echo Compiling engine_class.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/engine_class.cpp

collision.o: ../../src/uncategorized/collision.cpp \
 /home/godlike/.intel/Compiler/11.1/046/include/float.h \
 ../../src/uncategorized/collision.h ../../src/uncategorized/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/renderer/renderer.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h
	@echo Compiling collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/collision.cpp

handlers.o: ../../src/uncategorized/handlers.cpp \
 ../../src/uncategorized/handlers.h /usr/include/SDL/SDL.h \
 /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /usr/include/sys/types.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/bits/wordsize.h \
 /usr/include/bits/sigset.h /usr/include/time.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/wordsize.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/xlocale.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wchar.h \
 /usr/include/bits/wordsize.h /usr/include/ctype.h /usr/include/iconv.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_version.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h ../../src/uncategorized/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc
	@echo Compiling handlers.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/handlers.cpp

skybox.o: ../../src/uncategorized/skybox.cpp ../../src/uncategorized/skybox.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/texture.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4.1/limits ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/renderer/renderer.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/collision.h ../../src/uncategorized/object.h \
 ../../src/uncategorized/scene.h ../../src/uncategorized/spatial.h \
 ../../src/uncategorized/lights.h ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/material.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/smodel.h
	@echo Compiling skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/skybox.cpp

camera.o: ../../src/uncategorized/camera.cpp ../../src/uncategorized/camera.h \
 /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/sys/types.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/collision.h \
 ../../src/uncategorized/object.h ../../src/renderer/renderer.h \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/uncategorized/resource.h
	@echo Compiling camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/camera.cpp

object.o: ../../src/uncategorized/object.cpp /usr/include/stdio.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 ../../src/uncategorized/object.h ../../src/uncategorized/common.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/renderer/renderer.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/collision.h
	@echo Compiling object.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/object.cpp

shaders.o: ../../src/uncategorized/shaders.cpp /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/ostream /usr/include/c++/4.4.1/ios \
 /usr/include/c++/4.4.1/iosfwd /usr/include/c++/4.4.1/bits/stringfwd.h \
 /usr/include/c++/4.4.1/bits/postypes.h /usr/include/c++/4.4.1/cwchar \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/stdio.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/bits/istream.tcc /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/shaders.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/bits/codecvt.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/vector \
 /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/renderer/renderer.h \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/texture.h /usr/include/c++/4.4.1/limits \
 ../../src/tokenizer/parser.h
	@echo Compiling shaders.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/shaders.cpp

model.o: ../../src/uncategorized/model.cpp ../../src/uncategorized/model.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/tokenizer/parser.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/object.h ../../src/renderer/vbo.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h
	@echo Compiling model.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/model.cpp

hud.o: ../../src/uncategorized/hud.cpp /usr/include/stdio.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 ../../src/uncategorized/hud.h ../../src/uncategorized/common.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/renderer/renderer.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/c++/4.4.1/limits
	@echo Compiling hud.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/hud.cpp

map.o: ../../src/uncategorized/map.cpp /usr/include/c++/4.4.1/limits \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 ../../src/uncategorized/map.h ../../src/uncategorized/common.h \
 /usr/include/stdio.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/collision.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/object.h ../../src/renderer/vbo.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/tokenizer/parser.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h
	@echo Compiling map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/map.cpp

texture.o: ../../src/uncategorized/texture.cpp /usr/include/c++/4.4.1/fstream \
 /usr/include/c++/4.4.1/istream /usr/include/c++/4.4.1/ios \
 /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/stdio.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/SDL/SDL_image.h \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/stdlib.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wordsize.h \
 /usr/include/iconv.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_version.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h ../../src/uncategorized/texture.h \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h /usr/include/c++/4.4.1/limits \
 ../../src/uncategorized/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/common.h /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/renderer/renderer.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h
	@echo Compiling texture.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/texture.cpp

memory.o: ../../src/uncategorized/memory.cpp ../../src/uncategorized/memory.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wordsize.h \
 /usr/include/iconv.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_version.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h
	@echo Compiling memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/memory.cpp

common.o: ../../src/uncategorized/common.cpp /usr/include/stdio.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/string.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/c++/4.4.1/string /usr/include/c++/4.4.1/bits/stringfwd.h \
 /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/bits/postypes.h /usr/include/c++/4.4.1/cwchar \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/iosfwd /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/sched.h /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/uncategorized/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc
	@echo Compiling common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/common.cpp

smodel.o: ../../src/uncategorized/smodel.cpp /usr/include/c++/4.4.1/fstream \
 /usr/include/c++/4.4.1/istream /usr/include/c++/4.4.1/ios \
 /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/stdio.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/wordsize.h /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/c++/4.4.1/bits/ios_base.h \
 /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/common.h /usr/include/stdlib.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/iostream /usr/include/c++/4.4.1/vector \
 /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/material.h ../../src/uncategorized/resource.h \
 ../../src/uncategorized/object.h ../../src/renderer/vbo.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/stdint.h \
 /usr/include/bits/wordsize.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/renderer/renderer.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/tokenizer/parser.h
	@echo Compiling smodel.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/smodel.cpp

resource.o: ../../src/uncategorized/resource.cpp \
 ../../src/uncategorized/resource.h ../../src/uncategorized/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h /usr/include/c++/4.4.1/limits \
 ../../src/uncategorized/material.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h ../../src/uncategorized/mesh.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/object.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/smodel.h ../../src/uncategorized/model.h
	@echo Compiling resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/resource.cpp

particles.o: ../../src/uncategorized/particles.cpp \
 ../../src/uncategorized/particles.h ../../src/uncategorized/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/math/gmath.h /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 /usr/include/math.h /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/handlers.h \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wordsize.h \
 /usr/include/iconv.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_version.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h ../../src/uncategorized/object.h \
 ../../src/renderer/renderer.h /usr/include/GL/glew.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/uncategorized/shaders.h \
 /usr/include/c++/4.4.1/map /usr/include/c++/4.4.1/bits/stl_tree.h \
 /usr/include/c++/4.4.1/bits/stl_map.h /usr/include/c++/4.4.1/bits/stl_multimap.h \
 ../../src/uncategorized/resource.h
	@echo Compiling particles.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/particles.cpp

scene.o: ../../src/uncategorized/scene.cpp ../../src/uncategorized/scene.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/spatial.h \
 ../../src/uncategorized/collision.h ../../src/uncategorized/object.h \
 ../../src/uncategorized/lights.h ../../src/uncategorized/texture.h \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h /usr/include/c++/4.4.1/limits \
 ../../src/uncategorized/resource.h ../../src/uncategorized/camera.h \
 /usr/include/c++/4.4.1/cstring /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/mesh.h ../../src/uncategorized/material.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/smodel.h \
 ../../src/uncategorized/skybox.h
	@echo Compiling scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/scene.cpp

shader_parser.o: ../../src/uncategorized/shader_parser.cpp \
 /usr/include/c++/4.4.1/iomanip \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/iosfwd /usr/include/c++/4.4.1/bits/stringfwd.h \
 /usr/include/c++/4.4.1/bits/postypes.h /usr/include/c++/4.4.1/cwchar \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/stdio.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/xlocale.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/bits/wordsize.h /usr/include/sched.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/bits/setjmp.h /usr/include/bits/wordsize.h \
 /usr/include/bits/wordsize.h /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc \
 ../../src/uncategorized/shader_parser.h /usr/include/c++/4.4.1/limits \
 ../../src/uncategorized/common.h /usr/include/stdio.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/libio.h \
 /usr/include/_G_config.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/wchar.h /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4.1/sstream \
 /usr/include/c++/4.4.1/bits/sstream.tcc ../../src/tokenizer/parser.h
	@echo Compiling shader_parser.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/shader_parser.cpp

spatial.o: ../../src/uncategorized/spatial.cpp ../../src/uncategorized/spatial.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/collision.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/object.h
	@echo Compiling spatial.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/spatial.cpp

lights.o: ../../src/uncategorized/lights.cpp ../../src/uncategorized/lights.h \
 ../../src/uncategorized/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs-64.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/wordsize.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h /usr/include/_G_config.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/bits/wordsize.h /usr/include/xlocale.h /usr/include/sys/types.h \
 /usr/include/time.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/wordsize.h /usr/include/bits/sigset.h /usr/include/time.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/wordsize.h \
 /usr/include/alloca.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/common.h \
 /usr/include/string.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/fstream /usr/include/c++/4.4.1/istream \
 /usr/include/c++/4.4.1/ios /usr/include/c++/4.4.1/iosfwd \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4.1/bits/stringfwd.h /usr/include/c++/4.4.1/bits/postypes.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h /usr/include/bits/wchar.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/exception /usr/include/c++/4.4.1/bits/char_traits.h \
 /usr/include/c++/4.4.1/bits/stl_algobase.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/functexcept.h \
 /usr/include/c++/4.4.1/exception_defines.h \
 /usr/include/c++/4.4.1/bits/cpp_type_traits.h \
 /usr/include/c++/4.4.1/ext/type_traits.h \
 /usr/include/c++/4.4.1/ext/numeric_traits.h \
 /usr/include/c++/4.4.1/bits/stl_pair.h /usr/include/c++/4.4.1/bits/move.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/concept_check.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4.1/bits/stl_iterator.h /usr/include/c++/4.4.1/debug/debug.h \
 /usr/include/c++/4.4.1/cwchar /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/wchar.h \
 /usr/include/c++/4.4.1/bits/localefwd.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4.1/clocale /usr/include/locale.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4.1/bits/ios_base.h /usr/include/c++/4.4.1/ext/atomicity.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/time.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h /usr/include/bits/setjmp.h \
 /usr/include/bits/wordsize.h /usr/include/bits/wordsize.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/wordsize.h \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4.1/bits/locale_classes.h /usr/include/c++/4.4.1/string \
 /usr/include/c++/4.4.1/bits/allocator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4.1/ext/new_allocator.h /usr/include/c++/4.4.1/new \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/ostream_insert.h \
 /usr/include/c++/4.4.1/cxxabi-forced.h \
 /usr/include/c++/4.4.1/bits/stl_function.h \
 /usr/include/c++/4.4.1/backward/binders.h \
 /usr/include/c++/4.4.1/bits/basic_string.h \
 /usr/include/c++/4.4.1/initializer_list \
 /usr/include/c++/4.4.1/bits/basic_string.tcc \
 /usr/include/c++/4.4.1/bits/locale_classes.tcc /usr/include/c++/4.4.1/streambuf \
 /usr/include/c++/4.4.1/bits/streambuf.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.h \
 /usr/include/c++/4.4.1/bits/locale_facets.h /usr/include/c++/4.4.1/cwctype \
 /usr/include/wctype.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cctype \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4.1/bits/streambuf_iterator.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4.1/bits/locale_facets.tcc \
 /usr/include/c++/4.4.1/bits/basic_ios.tcc /usr/include/c++/4.4.1/ostream \
 /usr/include/c++/4.4.1/bits/ostream.tcc /usr/include/c++/4.4.1/bits/istream.tcc \
 /usr/include/c++/4.4.1/bits/codecvt.h /usr/include/c++/4.4.1/cstdio \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4.1/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4.1/cstdio /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/c++/4.4.1/bits/fstream.tcc /usr/include/c++/4.4.1/iostream \
 /usr/include/c++/4.4.1/vector /usr/include/c++/4.4.1/bits/stl_construct.h \
 /usr/include/c++/4.4.1/bits/stl_uninitialized.h \
 /usr/include/c++/4.4.1/bits/stl_vector.h \
 /usr/include/c++/4.4.1/bits/stl_bvector.h /usr/include/c++/4.4.1/bits/vector.tcc \
 ../../src/uncategorized/primitives.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/wordsize.h \
 /usr/include/bits/mathcalls.h /usr/include/bits/mathcalls.h \
 /usr/include/bits/mathcalls.h ../../src/math/gmath.h \
 /home/godlike/.intel/Compiler/11.1/046/include/math.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec2.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec4.h ../../src/math/vec4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec4.h ../../src/math/quat.h ../../src/math/quat.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/quat.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/axisang.h ../../src/math/euler.h ../../src/math/euler.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/euler.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/m_dflt_header.h ../../src/math/mat3.h \
 ../../src/math/mat4.h ../../src/math/mat4.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/mat4.h ../../src/math/m_misc.h ../../src/math/m_misc.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/m_misc.h \
 ../../src/uncategorized/engine_class.h ../../src/uncategorized/texture.h \
 /usr/include/GL/glew.h /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 /usr/include/stdint.h /usr/include/bits/wordsize.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h /usr/include/c++/4.4.1/limits \
 ../../src/uncategorized/resource.h ../../src/uncategorized/object.h \
 ../../src/uncategorized/camera.h /usr/include/c++/4.4.1/cstring \
 /usr/include/c++/4.4.1/cstddef \
 /home/godlike/.intel/Compiler/11.1/046/include/stddef.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/uncategorized/shaders.h /usr/include/c++/4.4.1/map \
 /usr/include/c++/4.4.1/bits/stl_tree.h /usr/include/c++/4.4.1/bits/stl_map.h \
 /usr/include/c++/4.4.1/bits/stl_multimap.h
	@echo Compiling lights.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/lights.cpp

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

