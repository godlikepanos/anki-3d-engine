CXX = g++
COMMONFLAGS = 
CFLAGS = $(COMMONFLAGS) -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_
PHFLAGS = $(CFLAGS) -x c++-header
LFLAGS = $(COMMONFLAGS) -L/usr/lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib,-Bstatic -lSDL -lSDL_image -Wl,-Bdynamic -lGL -lGLU -ljpeg -lGLEW
EXECUTABLE = AnKi.bin
INCPATH = -I../../src/math/ -I../../src/tokenizer/ -I../../src/uncategorized/ -I../../src/ -I../../src/renderer/ -I../../src/scene/ -I../../src/ui/ -I../../src/resources/ -I../../src/utility/ -I../../src/controllers/ -I../../../bullet_svn/src/ 
SOURCES = ../../src/tokenizer//scanner.cpp ../../src/uncategorized//input.cpp ../../src/uncategorized//engine_class.cpp ../../src/uncategorized//collision.cpp ../../src/uncategorized//skybox.cpp ../../src/uncategorized//map.cpp ../../src/uncategorized//memory.cpp ../../src/uncategorized//particles.cpp ../../src//main.cpp ../../src/renderer//renderer.cpp ../../src/renderer//r_is_shadows.cpp ../../src/renderer//r_is.cpp ../../src/renderer//r_ms_earlyz.cpp ../../src/renderer//r_pps.cpp ../../src/renderer//r_bs.cpp ../../src/renderer//r_dbg.cpp ../../src/renderer//r_ms.cpp ../../src/renderer//r_pps_lscatt.cpp ../../src/renderer//r_pps_hdr.cpp ../../src/renderer//r_bs2.cpp ../../src/renderer//r_pps_ssao.cpp ../../src/scene//skel_node.cpp ../../src/scene//skel_model_node.cpp ../../src/scene//camera.cpp ../../src/scene//node.cpp ../../src/scene//mesh_node.cpp ../../src/scene//light.cpp ../../src/scene//scene.cpp ../../src/ui//ui.cpp ../../src/resources//extension.cpp ../../src/resources//shader_prog.cpp ../../src/resources//material.cpp ../../src/resources//mesh.cpp ../../src/resources//skel_anim.cpp ../../src/resources//texture.cpp ../../src/resources//resource.cpp ../../src/resources//skeleton.cpp ../../src/resources//light_props.cpp ../../src/resources//shader_parser.cpp ../../src/utility//app.cpp ../../src/utility//common.cpp ../../src/utility//util.cpp ../../src/controllers//skel_anim_ctrl.cpp ../../src/controllers//controller.cpp 
OBJECTS = scanner.o input.o engine_class.o collision.o skybox.o map.o memory.o particles.o main.o renderer.o r_is_shadows.o r_is.o r_ms_earlyz.o r_pps.o r_bs.o r_dbg.o r_ms.o r_pps_lscatt.o r_pps_hdr.o r_bs2.o r_pps_ssao.o skel_node.o skel_model_node.o camera.o node.o mesh_node.o light.o scene.o ui.o extension.o shader_prog.o material.o mesh.o skel_anim.o texture.o resource.o skeleton.o light_props.o shader_parser.o app.o common.o util.o skel_anim_ctrl.o controller.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)
	@echo All Done!

scanner.o: ../../src/tokenizer//scanner.cpp \
 ../../src/tokenizer//scanner.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/tokenizer//scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/tokenizer//scanner.cpp -o scanner.o

input.o: ../../src/uncategorized//input.cpp \
 ../../src/uncategorized//input.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/utility/app.h \
 ../../src/utility/common.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h
	@echo Compiling ../../src/uncategorized//input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//input.cpp -o input.o

engine_class.o: ../../src/uncategorized//engine_class.cpp \
 ../../src/uncategorized//engine_class.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/uncategorized//engine_class.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//engine_class.cpp -o engine_class.o

collision.o: ../../src/uncategorized//collision.cpp \
 ../../src/uncategorized//collision.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/shader_prog.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h
	@echo Compiling ../../src/uncategorized//collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//collision.cpp -o collision.o

skybox.o: ../../src/uncategorized//skybox.cpp \
 ../../src/uncategorized//skybox.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources/texture.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h
	@echo Compiling ../../src/uncategorized//skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//skybox.cpp -o skybox.o

map.o: ../../src/uncategorized//map.cpp ../../src/uncategorized//map.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized//collision.h ../../src/math/gmath.h \
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
 ../../src/renderer/vbo.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/uncategorized/collision.h \
 ../../src/tokenizer/scanner.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/scanner.h ../../src/resources/resource.h \
 ../../src/scene/camera.h ../../src/scene/node.h
	@echo Compiling ../../src/uncategorized//map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//map.cpp -o map.o

memory.o: ../../src/uncategorized//memory.cpp \
 ../../src/uncategorized//memory.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/uncategorized//memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//memory.cpp -o memory.o

particles.o: ../../src/uncategorized//particles.cpp
	@echo Compiling ../../src/uncategorized//particles.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//particles.cpp -o particles.o

main.o: ../../src//main.cpp ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/input.h \
 ../../src/utility/app.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/ui/ui.h \
 ../../src/uncategorized/particles.h ../../src/resources/texture.h \
 ../../src/resources/mesh.h ../../src/renderer/vbo.h \
 ../../src/scene/light.h ../../src/scene/camera.h \
 ../../src/resources/material.h ../../src/resources/resource.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/tokenizer/scanner.h ../../src/uncategorized/map.h \
 ../../src/uncategorized/collision.h ../../src/scene/mesh_node.h \
 ../../src/scene/skel_model_node.h ../../src/scene/mesh_node.h \
 ../../src/resources/skel_anim.h ../../src/controllers/mesh_skel_ctrl.h \
 ../../src/controllers/controller.h \
 ../../src/controllers/skel_anim_ctrl.h ../../src/scene/skel_node.h \
 ../../src/controllers/controller.h ../../src/resources/light_props.h
	@echo Compiling ../../src//main.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src//main.cpp -o main.o

renderer.o: ../../src/renderer//renderer.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/texture.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer//r_private.h ../../src/utility/app.h
	@echo Compiling ../../src/renderer//renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//renderer.cpp -o renderer.o

r_is_shadows.o: ../../src/renderer//r_is_shadows.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/texture.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/resource.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/resources/material.h \
 ../../src/scene/mesh_node.h
	@echo Compiling ../../src/renderer//r_is_shadows.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_is_shadows.cpp -o r_is_shadows.o

r_is.o: ../../src/renderer//r_is.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/resources/mesh.h ../../src/renderer/vbo.h \
 ../../src/scene/light.h ../../src/scene/camera.h \
 ../../src/resources/resource.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/resources/light_props.h
	@echo Compiling ../../src/renderer//r_is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_is.cpp -o r_is.o

r_ms_earlyz.o: ../../src/renderer//r_ms_earlyz.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/resource.h \
 ../../src/resources/texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_ms_earlyz.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_ms_earlyz.cpp -o r_ms_earlyz.o

r_pps.o: ../../src/renderer//r_pps.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/resource.h \
 ../../src/resources/texture.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps.cpp -o r_pps.o

r_bs.o: ../../src/renderer//r_bs.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/resources/mesh.h ../../src/renderer/vbo.h \
 ../../src/renderer//r_private.h ../../src/resources/resource.h \
 ../../src/renderer//fbo.h ../../src/scene/mesh_node.h \
 ../../src/resources/material.h
	@echo Compiling ../../src/renderer//r_bs.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_bs.cpp -o r_bs.o

r_dbg.o: ../../src/renderer//r_dbg.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/scene/node.h ../../src/scene/skel_node.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/renderer//r_dbg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_dbg.cpp -o r_dbg.o

r_ms.o: ../../src/renderer//r_ms.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/resources/mesh.h ../../src/renderer/vbo.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h \
 ../../src/resources/material.h ../../src/scene/mesh_node.h
	@echo Compiling ../../src/renderer//r_ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_ms.cpp -o r_ms.o

r_pps_lscatt.o: ../../src/renderer//r_pps_lscatt.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/resource.h \
 ../../src/resources/texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_lscatt.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_lscatt.cpp -o r_pps_lscatt.o

r_pps_hdr.o: ../../src/renderer//r_pps_hdr.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/resource.h \
 ../../src/resources/texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_hdr.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_hdr.cpp -o r_pps_hdr.o

r_bs2.o: ../../src/renderer//r_bs2.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/resources/mesh.h ../../src/renderer/vbo.h \
 ../../src/renderer//r_private.h ../../src/resources/resource.h \
 ../../src/renderer//fbo.h ../../src/scene/mesh_node.h \
 ../../src/resources/material.h
	@echo Compiling ../../src/renderer//r_bs2.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_bs2.cpp -o r_bs2.o

r_pps_ssao.o: ../../src/renderer//r_pps_ssao.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/resource.h \
 ../../src/resources/texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_ssao.cpp -o r_pps_ssao.o

skel_node.o: ../../src/scene//skel_node.cpp ../../src/scene//skel_node.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/scene//node.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/controllers/controller.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/skel_anim.h \
 ../../src/resources/skeleton.h ../../src/controllers/skel_anim_ctrl.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/scene//skel_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//skel_node.cpp -o skel_node.o

skel_model_node.o: ../../src/scene//skel_model_node.cpp \
 ../../src/scene//skel_model_node.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/scene//mesh_node.h \
 ../../src/scene//node.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/material.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h \
 ../../src/scene//skel_node.h ../../src/controllers/controller.h \
 ../../src/controllers/mesh_skel_ctrl.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/scene//skel_model_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//skel_model_node.cpp -o skel_model_node.o

camera.o: ../../src/scene//camera.cpp ../../src/scene//camera.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/collision.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene//node.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h
	@echo Compiling ../../src/scene//camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//camera.cpp -o camera.o

node.o: ../../src/scene//node.cpp ../../src/scene//node.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/scene//node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//node.cpp -o node.o

mesh_node.o: ../../src/scene//mesh_node.cpp ../../src/scene//mesh_node.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/scene//node.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/material.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/resources/resource.h ../../src/resources/mesh.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/collision.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 ../../src/scene/camera.h ../../src/scene/node.h \
 ../../src/scene//skel_node.h ../../src/controllers/controller.h \
 ../../src/resources/skeleton.h ../../src/controllers/mesh_skel_ctrl.h \
 ../../src/controllers/controller.h \
 ../../src/controllers/skel_anim_ctrl.h
	@echo Compiling ../../src/scene//mesh_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//mesh_node.cpp -o mesh_node.o

light.o: ../../src/scene//light.cpp ../../src/scene//light.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/resources/texture.h ../../src/resources/resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene//node.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene//camera.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h ../../src/scene/camera.h \
 ../../src/resources/light_props.h
	@echo Compiling ../../src/scene//light.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//light.cpp -o light.o

scene.o: ../../src/scene//scene.cpp ../../src/scene//scene.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene//skel_node.h \
 ../../src/scene//node.h ../../src/controllers/controller.h \
 ../../src/scene//camera.h ../../src/uncategorized/collision.h \
 ../../src/scene//mesh_node.h ../../src/resources/material.h \
 ../../src/scene//light.h
	@echo Compiling ../../src/scene//scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//scene.cpp -o scene.o

ui.o: ../../src/ui//ui.cpp ../../src/ui//ui.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/renderer/renderer.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/texture.h \
 ../../src/resources/resource.h
	@echo Compiling ../../src/ui//ui.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/ui//ui.cpp -o ui.o

extension.o: ../../src/resources//extension.cpp \
 ../../src/resources//extension.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h
	@echo Compiling ../../src/resources//extension.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//extension.cpp -o extension.o

shader_prog.o: ../../src/resources//shader_prog.cpp \
 ../../src/resources//shader_prog.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/renderer/renderer.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/scene/node.h ../../src/resources//shader_parser.h \
 ../../src/resources//texture.h
	@echo Compiling ../../src/resources//shader_prog.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//shader_prog.cpp -o shader_prog.o

material.o: ../../src/resources//material.cpp \
 ../../src/resources//material.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/tokenizer/scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h \
 ../../src/resources//texture.h ../../src/resources//shader_prog.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h
	@echo Compiling ../../src/resources//material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//material.cpp -o material.o

mesh.o: ../../src/resources//mesh.cpp ../../src/resources//mesh.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/vbo.h \
 ../../src/resources//resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/shader_prog.h ../../src/resources/resource.h \
 ../../src/scene/camera.h ../../src/scene/node.h \
 ../../src/tokenizer/scanner.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/scanner.h
	@echo Compiling ../../src/resources//mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//mesh.cpp -o mesh.o

skel_anim.o: ../../src/resources//skel_anim.cpp \
 ../../src/resources//skel_anim.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h
	@echo Compiling ../../src/resources//skel_anim.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//skel_anim.cpp -o skel_anim.o

texture.o: ../../src/resources//texture.cpp \
 ../../src/resources//texture.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/renderer/renderer.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/resources/resource.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h
	@echo Compiling ../../src/resources//texture.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//texture.cpp -o texture.o

resource.o: ../../src/resources//resource.cpp \
 ../../src/resources//resource.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/resources//texture.h ../../src/resources//material.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources//shader_prog.h \
 ../../src/resources//mesh.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/collision.h ../../src/resources//skel_anim.h \
 ../../src/resources//light_props.h
	@echo Compiling ../../src/resources//resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//resource.cpp -o resource.o

skeleton.o: ../../src/resources//skeleton.cpp \
 ../../src/resources//skeleton.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/tokenizer/scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h
	@echo Compiling ../../src/resources//skeleton.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//skeleton.cpp -o skeleton.o

light_props.o: ../../src/resources//light_props.cpp \
 ../../src/resources//light_props.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/scanner.h ../../src/resources//texture.h
	@echo Compiling ../../src/resources//light_props.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//light_props.cpp -o light_props.o

shader_parser.o: ../../src/resources//shader_parser.cpp \
 ../../src/resources//shader_parser.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/tokenizer/scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/scanner.h \
 ../../src/utility/util.h ../../src/utility/common.h
	@echo Compiling ../../src/resources//shader_parser.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//shader_parser.cpp -o shader_parser.o

app.o: ../../src/utility//app.cpp ../../src/utility//app.h \
 ../../src/utility//common.h ../../src/uncategorized/memory.h \
 ../../src/utility/common.h
	@echo Compiling ../../src/utility//app.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//app.cpp -o app.o

common.o: ../../src/utility//common.cpp ../../src/utility//common.h \
 ../../src/uncategorized/memory.h ../../src/utility/common.h
	@echo Compiling ../../src/utility//common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//common.cpp -o common.o

util.o: ../../src/utility//util.cpp ../../src/utility//util.h \
 ../../src/utility//common.h ../../src/uncategorized/memory.h \
 ../../src/utility/common.h
	@echo Compiling ../../src/utility//util.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//util.cpp -o util.o

skel_anim_ctrl.o: ../../src/controllers//skel_anim_ctrl.cpp \
 ../../src/controllers//skel_anim_ctrl.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/controllers//controller.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
 ../../src/controllers/controller.h ../../src/resources/skeleton.h \
 ../../src/renderer/renderer.h ../../src/resources/shader_prog.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h
	@echo Compiling ../../src/controllers//skel_anim_ctrl.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/controllers//skel_anim_ctrl.cpp -o skel_anim_ctrl.o

controller.o: ../../src/controllers//controller.cpp \
 ../../src/controllers//controller.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/texture.h \
 ../../src/resources/resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
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
	@echo Compiling ../../src/controllers//controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/controllers//controller.cpp -o controller.o

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

