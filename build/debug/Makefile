CXX = g++
COMMONFLAGS = 
CFLAGS = $(COMMONFLAGS) -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -D_DEBUG_ -D_TERMINAL_COLORING_ -D_PLATFORM_LINUX_ -I../../../glew/include
PHFLAGS = $(CFLAGS) -x c++-header
LFLAGS = $(COMMONFLAGS) -lGL -lGLU -lSDL_image `sdl-config --static-libs` -ljpeg -lGLEW -L../../../glew/lib
EXECUTABLE = AnKi
INCPATH = -I../../src/math/ -I../../src/tokenizer/ -I../../src/uncategorized/ -I../../src/ -I../../src/renderer/ -I../../src/scene/ -I../../src/resources/ -I../../src/utility/ 
SOURCES = ../../src/tokenizer/scanner.cpp ../../src/uncategorized/input.cpp ../../src/uncategorized/engine_class.cpp ../../src/uncategorized/collision.cpp ../../src/uncategorized/skybox.cpp ../../src/uncategorized/hud.cpp ../../src/uncategorized/map.cpp ../../src/uncategorized/memory.cpp ../../src/uncategorized/particles.cpp ../../src/main.cpp ../../src/renderer/renderer.cpp ../../src/renderer/r_is_shadows.cpp ../../src/renderer/r_is.cpp ../../src/renderer/r_ms_earlyz.cpp ../../src/renderer/r_pps.cpp ../../src/renderer/r_bs.cpp ../../src/renderer/r_dbg.cpp ../../src/renderer/r_ms.cpp ../../src/renderer/r_pps_lscatt.cpp ../../src/renderer/r_pps_hdr.cpp ../../src/renderer/r_pps_ssao.cpp ../../src/scene/controller.cpp ../../src/scene/skel_node.cpp ../../src/scene/skel_model_node.cpp ../../src/scene/camera.cpp ../../src/scene/node.cpp ../../src/scene/skel_anim_controller.cpp ../../src/scene/mesh_node.cpp ../../src/scene/light.cpp ../../src/scene/scene.cpp ../../src/resources/shader_prog.cpp ../../src/resources/material.cpp ../../src/resources/mesh.cpp ../../src/resources/skel_anim.cpp ../../src/resources/texture.cpp ../../src/resources/resource.cpp ../../src/resources/skeleton.cpp ../../src/resources/light_props.cpp ../../src/resources/shader_parser.cpp ../../src/utility/app.cpp ../../src/utility/common.cpp ../../src/utility/util.cpp 
OBJECTS = scanner.o input.o engine_class.o collision.o skybox.o hud.o map.o memory.o particles.o main.o renderer.o r_is_shadows.o r_is.o r_ms_earlyz.o r_pps.o r_bs.o r_dbg.o r_ms.o r_pps_lscatt.o r_pps_hdr.o r_pps_ssao.o controller.o skel_node.o skel_model_node.o camera.o node.o skel_anim_controller.o mesh_node.o light.o scene.o shader_prog.o material.o mesh.o skel_anim.o texture.o resource.o skeleton.o light_props.o shader_parser.o app.o common.o util.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	@$(CXX) $(LFLAGS) $(OBJECTS) -o $(EXECUTABLE)
	@echo All Done!

scanner.o: ../../src/tokenizer/scanner.cpp /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/wchar.h /usr/include/stdio.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h /usr/include/xlocale.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/sigset.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/string.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 /usr/include/c++/4.4/sstream /usr/include/c++/4.4/bits/sstream.tcc \
 /usr/include/c++/4.4/iomanip ../../src/tokenizer/scanner.h \
 ../../src/utility/common.h /usr/include/stdlib.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/c++/4.4/iostream /usr/include/c++/4.4/vector \
 /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h
	@echo Compiling scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/tokenizer/scanner.cpp

input.o: ../../src/uncategorized/input.cpp \
 ../../src/uncategorized/input.h /usr/include/SDL/SDL.h \
 /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /usr/include/sys/types.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/bits/sigset.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/xlocale.h \
 /usr/include/alloca.h /usr/include/string.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/bits/wchar.h \
 /usr/include/ctype.h /usr/include/iconv.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_audio.h \
 /usr/include/SDL/SDL_error.h /usr/include/SDL/SDL_endian.h \
 /usr/include/SDL/SDL_mutex.h /usr/include/SDL/SDL_thread.h \
 /usr/include/SDL/SDL_rwops.h /usr/include/SDL/SDL_cdrom.h \
 /usr/include/SDL/SDL_cpuinfo.h /usr/include/SDL/SDL_events.h \
 /usr/include/SDL/SDL_active.h /usr/include/SDL/SDL_keyboard.h \
 /usr/include/SDL/SDL_keysym.h /usr/include/SDL/SDL_mouse.h \
 /usr/include/SDL/SDL_video.h /usr/include/SDL/SDL_joystick.h \
 /usr/include/SDL/SDL_quit.h /usr/include/SDL/SDL_loadso.h \
 /usr/include/SDL/SDL_timer.h /usr/include/SDL/SDL_version.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/utility/app.h ../../src/utility/common.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../../glew/include/GL/glew.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h
	@echo Compiling input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/input.cpp

engine_class.o: ../../src/uncategorized/engine_class.cpp \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h /usr/include/c++/4.4/cstring \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/cstddef /usr/include/string.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/bits/wchar.h /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h
	@echo Compiling engine_class.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/engine_class.cpp

collision.o: ../../src/uncategorized/collision.cpp \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/float.h \
 ../../src/uncategorized/collision.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h
	@echo Compiling collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/collision.cpp

skybox.o: ../../src/uncategorized/skybox.cpp \
 ../../src/uncategorized/skybox.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/resources/texture.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4/limits ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/math/gmath.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h \
 /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/resource.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h
	@echo Compiling skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/skybox.cpp

hud.o: ../../src/uncategorized/hud.cpp /usr/include/stdio.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 ../../src/uncategorized/hud.h ../../src/utility/common.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/renderer/renderer.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/resources/resource.h
	@echo Compiling hud.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/hud.cpp

map.o: ../../src/uncategorized/map.cpp /usr/include/c++/4.4/limits \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 ../../src/uncategorized/map.h ../../src/utility/common.h \
 /usr/include/stdio.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/uncategorized/collision.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/uncategorized/collision.h ../../src/tokenizer/scanner.h \
 /usr/include/c++/4.4/sstream /usr/include/c++/4.4/bits/sstream.tcc \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h \
 ../../src/resources/resource.h ../../src/scene/camera.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h
	@echo Compiling map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/map.cpp

memory.o: ../../src/uncategorized/memory.cpp \
 ../../src/uncategorized/memory.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h \
 /usr/include/SDL/SDL_stdinc.h /usr/include/SDL/SDL_config.h \
 /usr/include/SDL/SDL_platform.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/iconv.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/SDL_version.h
	@echo Compiling memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/memory.cpp

particles.o: ../../src/uncategorized/particles.cpp
	@echo Compiling particles.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized/particles.cpp

main.o: ../../src/main.cpp /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/ostream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/xlocale.h /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/sigset.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc \
 /usr/include/c++/4.4/bits/ostream.tcc /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/bits/istream.tcc /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/typeinfo \
 ../../src/utility/common.h /usr/include/stdlib.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/sys/types.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/vector \
 /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/uncategorized/input.h /usr/include/SDL/SDL.h \
 /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_stdinc.h \
 /usr/include/SDL/SDL_config.h /usr/include/SDL/SDL_platform.h \
 /usr/include/strings.h /usr/include/inttypes.h /usr/include/stdint.h \
 /usr/include/iconv.h /usr/include/SDL/begin_code.h \
 /usr/include/SDL/close_code.h /usr/include/SDL/SDL_audio.h \
 /usr/include/SDL/SDL_error.h /usr/include/SDL/SDL_endian.h \
 /usr/include/SDL/SDL_mutex.h /usr/include/SDL/SDL_thread.h \
 /usr/include/SDL/SDL_rwops.h /usr/include/SDL/SDL_cdrom.h \
 /usr/include/SDL/SDL_cpuinfo.h /usr/include/SDL/SDL_events.h \
 /usr/include/SDL/SDL_active.h /usr/include/SDL/SDL_keyboard.h \
 /usr/include/SDL/SDL_keysym.h /usr/include/SDL/SDL_mouse.h \
 /usr/include/SDL/SDL_video.h /usr/include/SDL/SDL_joystick.h \
 /usr/include/SDL/SDL_quit.h /usr/include/SDL/SDL_loadso.h \
 /usr/include/SDL/SDL_timer.h /usr/include/SDL/SDL_version.h \
 ../../src/utility/app.h ../../src/utility/common.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/renderer/renderer.h \
 ../../../glew/include/GL/glew.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/uncategorized/hud.h ../../src/uncategorized/particles.h \
 ../../src/resources/texture.h /usr/include/c++/4.4/limits \
 ../../src/resources/mesh.h ../../src/renderer/vbo.h \
 ../../src/scene/light.h ../../src/scene/camera.h \
 ../../src/resources/material.h ../../src/resources/resource.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4/sstream \
 /usr/include/c++/4.4/bits/sstream.tcc ../../src/uncategorized/map.h \
 ../../src/uncategorized/collision.h ../../src/scene/mesh_node.h \
 ../../src/scene/skel_model_node.h ../../src/scene/mesh_node.h \
 ../../src/resources/skel_anim.h ../../src/scene/skel_controller.h \
 ../../src/scene/controller.h ../../src/scene/skel_anim_controller.h \
 ../../src/scene/skel_node.h ../../src/resources/light_props.h
	@echo Compiling main.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/main.cpp

renderer.o: ../../src/renderer/renderer.cpp /usr/include/stdlib.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/xlocale.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h \
 /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/jpeglib.h /usr/include/jconfig.h /usr/include/jmorecfg.h \
 ../../src/renderer/renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h /usr/include/string.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer/r_private.h
	@echo Compiling renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/renderer.cpp

r_is_shadows.o: ../../src/renderer/r_is_shadows.cpp \
 ../../src/renderer/renderer.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/resource.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h \
 ../../src/resources/material.h ../../src/scene/mesh_node.h
	@echo Compiling r_is_shadows.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_is_shadows.cpp

r_is.o: ../../src/renderer/r_is.cpp ../../src/renderer/renderer.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../src/scene/light.h \
 ../../src/scene/camera.h ../../src/resources/resource.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h \
 ../../src/resources/light_props.h
	@echo Compiling r_is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_is.cpp

r_ms_earlyz.o: ../../src/renderer/r_ms_earlyz.cpp \
 ../../src/renderer/renderer.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/resource.h \
 ../../src/resources/texture.h /usr/include/c++/4.4/limits \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_ms_earlyz.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_ms_earlyz.cpp

r_pps.o: ../../src/renderer/r_pps.cpp ../../src/renderer/renderer.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/resource.h \
 ../../src/resources/texture.h /usr/include/c++/4.4/limits \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps.cpp

r_bs.o: ../../src/renderer/r_bs.cpp ../../src/renderer/renderer.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../src/renderer/r_private.h \
 ../../src/resources/resource.h ../../src/renderer/fbo.h \
 ../../src/scene/mesh_node.h ../../src/resources/material.h
	@echo Compiling r_bs.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_bs.cpp

r_dbg.o: ../../src/renderer/r_dbg.cpp ../../src/renderer/renderer.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/scene/node.h \
 ../../src/scene/skel_node.h ../../src/scene/controller.h
	@echo Compiling r_dbg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_dbg.cpp

r_ms.o: ../../src/renderer/r_ms.cpp ../../src/renderer/renderer.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 /usr/include/c++/4.4/limits ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../src/renderer/r_private.h \
 ../../src/renderer/fbo.h ../../src/resources/material.h \
 ../../src/scene/mesh_node.h
	@echo Compiling r_ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_ms.cpp

r_pps_lscatt.o: ../../src/renderer/r_pps_lscatt.cpp \
 ../../src/renderer/renderer.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/resource.h \
 ../../src/resources/texture.h /usr/include/c++/4.4/limits \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_pps_lscatt.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps_lscatt.cpp

r_pps_hdr.o: ../../src/renderer/r_pps_hdr.cpp \
 ../../src/renderer/renderer.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/resource.h \
 ../../src/resources/texture.h /usr/include/c++/4.4/limits \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_pps_hdr.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps_hdr.cpp

r_pps_ssao.o: ../../src/renderer/r_pps_ssao.cpp \
 ../../src/renderer/renderer.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/resource.h \
 ../../src/resources/texture.h /usr/include/c++/4.4/limits \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer/r_private.h ../../src/renderer/fbo.h
	@echo Compiling r_pps_ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer/r_pps_ssao.cpp

controller.o: ../../src/scene/controller.cpp ../../src/scene/controller.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/texture.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4/limits ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/math/gmath.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h \
 /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h
	@echo Compiling controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/controller.cpp

skel_node.o: ../../src/scene/skel_node.cpp ../../src/scene/skel_node.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/controller.h \
 ../../src/renderer/renderer.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/resources/skel_anim.h \
 ../../src/resources/skeleton.h ../../src/scene/skel_anim_controller.h
	@echo Compiling skel_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/skel_node.cpp

skel_model_node.o: ../../src/scene/skel_model_node.cpp \
 ../../src/scene/skel_model_node.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/scene/mesh_node.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4/sstream \
 /usr/include/c++/4.4/bits/sstream.tcc ../../src/scene/skel_node.h \
 ../../src/scene/controller.h ../../src/scene/skel_controller.h
	@echo Compiling skel_model_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/skel_model_node.cpp

camera.o: ../../src/scene/camera.cpp ../../src/scene/camera.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/uncategorized/collision.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/renderer/renderer.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h
	@echo Compiling camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/camera.cpp

node.o: ../../src/scene/node.cpp /usr/include/c++/4.4/algorithm \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/cstddef \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/stl_algo.h \
 /usr/include/c++/4.4/cstdlib /usr/include/stdlib.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/xlocale.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h /usr/include/c++/4.4/bits/algorithmfwd.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/stl_heap.h \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_construct.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/stl_uninitialized.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 ../../src/uncategorized/memory.h /usr/include/string.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/bits/wchar.h /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/controller.h
	@echo Compiling node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/node.cpp

skel_anim_controller.o: ../../src/scene/skel_anim_controller.cpp \
 ../../src/scene/skel_anim_controller.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/scene/controller.h ../../src/math/gmath.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h \
 /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/skel_anim.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/skel_node.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/resources/skeleton.h \
 ../../src/renderer/renderer.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h
	@echo Compiling skel_anim_controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/skel_anim_controller.cpp

mesh_node.o: ../../src/scene/mesh_node.cpp ../../src/scene/mesh_node.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/resource.h ../../src/uncategorized/collision.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/scene/camera.h \
 ../../src/resources/material.h ../../src/scene/skel_node.h \
 ../../src/scene/controller.h ../../src/resources/skeleton.h \
 ../../src/scene/skel_controller.h ../../src/scene/skel_anim_controller.h
	@echo Compiling mesh_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/mesh_node.cpp

light.o: ../../src/scene/light.cpp ../../src/scene/light.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/resources/texture.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4/limits ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/scene/camera.h \
 ../../src/resources/light_props.h
	@echo Compiling light.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/light.cpp

scene.o: ../../src/scene/scene.cpp /usr/include/c++/4.4/algorithm \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/cstddef \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/stl_algo.h \
 /usr/include/c++/4.4/cstdlib /usr/include/stdlib.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/xlocale.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h /usr/include/c++/4.4/bits/algorithmfwd.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/stl_heap.h \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_construct.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/stl_uninitialized.h ../../src/scene/scene.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/libio.h \
 /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 ../../src/uncategorized/memory.h /usr/include/string.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/bits/wchar.h /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h /usr/include/c++/4.4/limits \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/skel_node.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/scene/controller.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/mesh_node.h ../../src/scene/light.h \
 ../../src/resources/material.h
	@echo Compiling scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene/scene.cpp

shader_prog.o: ../../src/resources/shader_prog.cpp \
 ../../src/resources/shader_prog.h ../../../glew/include/GL/glew.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/stdint.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h /usr/include/bits/wchar.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/cstddef /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/libio.h \
 /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/renderer/renderer.h ../../src/math/gmath.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h \
 /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h \
 ../../src/resources/shader_parser.h /usr/include/c++/4.4/limits \
 ../../src/resources/texture.h
	@echo Compiling shader_prog.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/shader_prog.cpp

material.o: ../../src/resources/material.cpp /usr/include/string.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/xlocale.h ../../src/resources/material.h \
 ../../src/utility/common.h /usr/include/stdio.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/bits/sigset.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/alloca.h \
 ../../src/uncategorized/memory.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/tokenizer/scanner.h \
 /usr/include/c++/4.4/sstream /usr/include/c++/4.4/bits/sstream.tcc \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h \
 ../../src/resources/texture.h ../../../glew/include/GL/glew.h \
 /usr/include/stdint.h /usr/include/GL/glu.h /usr/include/GL/gl.h \
 /usr/include/c++/4.4/limits ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h
	@echo Compiling material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/material.cpp

mesh.o: ../../src/resources/mesh.cpp ../../src/resources/mesh.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/vbo.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h /usr/include/c++/4.4/map \
 /usr/include/c++/4.4/bits/stl_tree.h /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/scene/camera.h \
 ../../src/scene/node.h /usr/include/c++/4.4/memory \
 /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h ../../src/tokenizer/scanner.h \
 /usr/include/c++/4.4/sstream /usr/include/c++/4.4/bits/sstream.tcc \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h
	@echo Compiling mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/mesh.cpp

skel_anim.o: ../../src/resources/skel_anim.cpp \
 ../../src/resources/skel_anim.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/scanner.h \
 /usr/include/c++/4.4/sstream /usr/include/c++/4.4/bits/sstream.tcc \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h
	@echo Compiling skel_anim.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/skel_anim.cpp

texture.o: ../../src/resources/texture.cpp /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/wchar.h /usr/include/stdio.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h /usr/include/xlocale.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/bits/types.h \
 /usr/include/bits/typesizes.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h \
 /usr/include/bits/sched.h /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/sigset.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/SDL/SDL_image.h \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h \
 /usr/include/SDL/SDL_stdinc.h /usr/include/SDL/SDL_config.h \
 /usr/include/SDL/SDL_platform.h /usr/include/sys/types.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/sys/sysmacros.h /usr/include/stdlib.h \
 /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h \
 /usr/include/alloca.h /usr/include/string.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/stdint.h /usr/include/iconv.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/SDL_version.h ../../src/resources/texture.h \
 ../../../glew/include/GL/glew.h /usr/include/GL/glu.h \
 /usr/include/GL/gl.h /usr/include/c++/4.4/limits \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 /usr/include/c++/4.4/iostream /usr/include/c++/4.4/vector \
 /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/renderer/renderer.h ../../src/math/gmath.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h \
 /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 /usr/include/c++/4.4/memory /usr/include/c++/4.4/bits/stl_tempbuf.h \
 /usr/include/c++/4.4/bits/stl_raw_storage_iter.h \
 /usr/include/c++/4.4/backward/auto_ptr.h
	@echo Compiling texture.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/texture.cpp

resource.o: ../../src/resources/resource.cpp \
 ../../src/resources/resource.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/resources/texture.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h /usr/include/c++/4.4/limits \
 ../../src/resources/material.h ../../src/math/gmath.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/shader_prog.h \
 /usr/include/c++/4.4/map /usr/include/c++/4.4/bits/stl_tree.h \
 /usr/include/c++/4.4/bits/stl_map.h \
 /usr/include/c++/4.4/bits/stl_multimap.h ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/collision.h \
 ../../src/resources/skel_anim.h ../../src/resources/light_props.h
	@echo Compiling resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/resource.cpp

skeleton.o: ../../src/resources/skeleton.cpp \
 ../../src/resources/skeleton.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/tokenizer/scanner.h \
 /usr/include/c++/4.4/sstream /usr/include/c++/4.4/bits/sstream.tcc \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h
	@echo Compiling skeleton.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/skeleton.cpp

light_props.o: ../../src/resources/light_props.cpp \
 ../../src/resources/light_props.h ../../src/utility/common.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 /usr/include/string.h /usr/include/c++/4.4/fstream \
 /usr/include/c++/4.4/istream /usr/include/c++/4.4/ios \
 /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4/sstream \
 /usr/include/c++/4.4/bits/sstream.tcc ../../src/resources/texture.h \
 ../../../glew/include/GL/glew.h /usr/include/stdint.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h /usr/include/c++/4.4/limits
	@echo Compiling light_props.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/light_props.cpp

shader_parser.o: ../../src/resources/shader_parser.cpp \
 /usr/include/c++/4.4/iomanip \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/iosfwd /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/wchar.h /usr/include/stdio.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/wchar.h /usr/include/xlocale.h \
 /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/sched.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/time.h /usr/include/bits/sched.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/sigset.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/setjmp.h \
 /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/environments.h /usr/include/bits/confname.h \
 /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/locale_classes.h \
 /usr/include/c++/4.4/string /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 ../../src/resources/shader_parser.h /usr/include/c++/4.4/limits \
 ../../src/utility/common.h /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/sys/types.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/sys/sysmacros.h /usr/include/alloca.h \
 ../../src/uncategorized/memory.h /usr/include/string.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/streambuf \
 /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 ../../src/tokenizer/scanner.h /usr/include/c++/4.4/sstream \
 /usr/include/c++/4.4/bits/sstream.tcc ../../src/tokenizer/parser.h \
 ../../src/tokenizer/scanner.h ../../src/utility/util.h \
 ../../src/utility/common.h
	@echo Compiling shader_parser.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources/shader_parser.cpp

app.o: ../../src/utility/app.cpp ../../../glew/include/GL/glew.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/stdint.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h /usr/include/bits/wchar.h \
 /usr/include/GL/glu.h /usr/include/GL/gl.h ../../src/utility/app.h \
 /usr/include/SDL/SDL.h /usr/include/SDL/SDL_main.h \
 /usr/include/SDL/SDL_stdinc.h /usr/include/SDL/SDL_config.h \
 /usr/include/SDL/SDL_platform.h /usr/include/sys/types.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/bits/byteswap.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/bits/sigset.h \
 /usr/include/bits/time.h /usr/include/sys/sysmacros.h \
 /usr/include/bits/pthreadtypes.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/xlocale.h \
 /usr/include/alloca.h /usr/include/string.h /usr/include/strings.h \
 /usr/include/inttypes.h /usr/include/ctype.h /usr/include/iconv.h \
 /usr/include/SDL/begin_code.h /usr/include/SDL/close_code.h \
 /usr/include/SDL/SDL_audio.h /usr/include/SDL/SDL_error.h \
 /usr/include/SDL/SDL_endian.h /usr/include/SDL/SDL_mutex.h \
 /usr/include/SDL/SDL_thread.h /usr/include/SDL/SDL_rwops.h \
 /usr/include/SDL/SDL_cdrom.h /usr/include/SDL/SDL_cpuinfo.h \
 /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h \
 /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h \
 /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h \
 /usr/include/SDL/SDL_joystick.h /usr/include/SDL/SDL_quit.h \
 /usr/include/SDL/SDL_loadso.h /usr/include/SDL/SDL_timer.h \
 /usr/include/SDL/SDL_version.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/utility/common.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/c++/4.4/exception \
 /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h
	@echo Compiling app.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility/app.cpp

common.o: ../../src/utility/common.cpp /usr/include/stdio.h \
 /usr/include/features.h /usr/include/bits/predefs.h \
 /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
 /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h /usr/include/c++/4.4/cstring \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/cstddef /usr/include/string.h \
 /usr/include/c++/4.4/string /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/postypes.h \
 /usr/include/c++/4.4/cwchar /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/cctype /usr/include/ctype.h \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/huge_valf.h \
 /usr/include/bits/huge_vall.h /usr/include/bits/inf.h \
 /usr/include/bits/nan.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/utility/common.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/bits/locale_classes.h \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h
	@echo Compiling common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility/common.cpp

util.o: ../../src/utility/util.cpp ../../src/utility/util.h \
 ../../src/utility/common.h /usr/include/stdio.h /usr/include/features.h \
 /usr/include/bits/predefs.h /usr/include/sys/cdefs.h \
 /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
 /usr/include/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stddef.h \
 /usr/include/bits/types.h /usr/include/bits/typesizes.h \
 /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
 /usr/lib/gcc/x86_64-linux-gnu/4.4.1/include/stdarg.h \
 /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
 /usr/include/stdlib.h /usr/include/bits/waitflags.h \
 /usr/include/bits/waitstatus.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/bits/byteswap.h \
 /usr/include/xlocale.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/bits/time.h \
 /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h \
 /usr/include/alloca.h ../../src/uncategorized/memory.h \
 ../../src/utility/common.h /usr/include/string.h \
 /usr/include/c++/4.4/fstream /usr/include/c++/4.4/istream \
 /usr/include/c++/4.4/ios /usr/include/c++/4.4/iosfwd \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++config.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/os_defines.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/cpu_defines.h \
 /usr/include/c++/4.4/bits/stringfwd.h \
 /usr/include/c++/4.4/bits/postypes.h /usr/include/c++/4.4/cwchar \
 /usr/include/c++/4.4/cstddef /usr/include/bits/wchar.h \
 /usr/include/c++/4.4/exception /usr/include/c++/4.4/bits/char_traits.h \
 /usr/include/c++/4.4/bits/stl_algobase.h \
 /usr/include/c++/4.4/bits/functexcept.h \
 /usr/include/c++/4.4/exception_defines.h \
 /usr/include/c++/4.4/bits/cpp_type_traits.h \
 /usr/include/c++/4.4/ext/type_traits.h \
 /usr/include/c++/4.4/ext/numeric_traits.h \
 /usr/include/c++/4.4/bits/stl_pair.h /usr/include/c++/4.4/bits/move.h \
 /usr/include/c++/4.4/bits/concept_check.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_types.h \
 /usr/include/c++/4.4/bits/stl_iterator_base_funcs.h \
 /usr/include/c++/4.4/bits/stl_iterator.h \
 /usr/include/c++/4.4/debug/debug.h /usr/include/c++/4.4/bits/localefwd.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++locale.h \
 /usr/include/c++/4.4/clocale /usr/include/locale.h \
 /usr/include/bits/locale.h /usr/include/c++/4.4/cctype \
 /usr/include/ctype.h /usr/include/c++/4.4/bits/ios_base.h \
 /usr/include/c++/4.4/ext/atomicity.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/gthr-default.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/signal.h /usr/include/bits/setjmp.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/environments.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/atomic_word.h \
 /usr/include/c++/4.4/bits/locale_classes.h /usr/include/c++/4.4/string \
 /usr/include/c++/4.4/bits/allocator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++allocator.h \
 /usr/include/c++/4.4/ext/new_allocator.h /usr/include/c++/4.4/new \
 /usr/include/c++/4.4/bits/ostream_insert.h \
 /usr/include/c++/4.4/cxxabi-forced.h \
 /usr/include/c++/4.4/bits/stl_function.h \
 /usr/include/c++/4.4/backward/binders.h \
 /usr/include/c++/4.4/bits/basic_string.h \
 /usr/include/c++/4.4/initializer_list \
 /usr/include/c++/4.4/bits/basic_string.tcc \
 /usr/include/c++/4.4/bits/locale_classes.tcc \
 /usr/include/c++/4.4/streambuf /usr/include/c++/4.4/bits/streambuf.tcc \
 /usr/include/c++/4.4/bits/basic_ios.h \
 /usr/include/c++/4.4/bits/locale_facets.h /usr/include/c++/4.4/cwctype \
 /usr/include/wctype.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_base.h \
 /usr/include/c++/4.4/bits/streambuf_iterator.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/ctype_inline.h \
 /usr/include/c++/4.4/bits/locale_facets.tcc \
 /usr/include/c++/4.4/bits/basic_ios.tcc /usr/include/c++/4.4/ostream \
 /usr/include/c++/4.4/bits/ostream.tcc \
 /usr/include/c++/4.4/bits/istream.tcc \
 /usr/include/c++/4.4/bits/codecvt.h /usr/include/c++/4.4/cstdio \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/basic_file.h \
 /usr/include/c++/4.4/x86_64-linux-gnu/bits/c++io.h \
 /usr/include/c++/4.4/bits/fstream.tcc /usr/include/c++/4.4/iostream \
 /usr/include/c++/4.4/vector /usr/include/c++/4.4/bits/stl_construct.h \
 /usr/include/c++/4.4/bits/stl_uninitialized.h \
 /usr/include/c++/4.4/bits/stl_vector.h \
 /usr/include/c++/4.4/bits/stl_bvector.h \
 /usr/include/c++/4.4/bits/vector.tcc /usr/include/malloc.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/huge_valf.h /usr/include/bits/huge_vall.h \
 /usr/include/bits/inf.h /usr/include/bits/nan.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
	@echo Compiling util.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility/util.cpp

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

