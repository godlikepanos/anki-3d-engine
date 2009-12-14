#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "common.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "math.h"
#include "shaders.h"
#include "camera.h"

namespace r { // begin namespace


extern uint w, h, frames_num;
extern float aspect_ratio;

extern int  max_color_atachments;

extern bool show_axis, show_fnormals, show_vnormals, show_lights, show_skeletons, show_cameras, show_bvolumes;

extern void Init();
extern void PrepareNextFrame();
extern void PrintLastError();
extern void RenderGrid();
extern void RenderSphere( float radius, int precision );
extern void RenderCube( bool cols = false, float size = 1.0f );
extern void RenderQuad( float w, float h );
inline const char* GetStdShaderPreprocDefines() { extern char* std_shader_preproc_defines; return std_shader_preproc_defines; }

extern void SetGLState_Wireframe();
extern void SetGLState_WireframeDotted();
extern void SetGLState_Solid();
extern void SetGLState_AlphaSolid();

// ogl and glu wrappers
inline void MultMatrix( const mat4_t& m4 ) { glMultMatrixf( &(m4.Transposed())(0,0) ); }
inline void LoadMatrix( const mat4_t& m4 ) { glLoadMatrixf( &(m4.Transposed())(0,0) ); }
inline void Color3( const vec3_t& v ) { glColor3fv( &((vec3_t&)v)[0] ); }
inline void Color4( const vec4_t& v ) { glColor4fv( &((vec4_t&)v)[0] ); }
inline void NoShaders() { shader_prog_t::UnBind(); }
extern bool UnProject( float winX, float winY, float winZ, const mat4_t& modelview_mat, const mat4_t& projection_mat, const int* view, float& objX, float& objY, float& objZ );

// Matrix stuff
extern void SetProjectionMatrix( const camera_t& cam );
extern void SetViewMatrix( const camera_t& cam );
inline void SetProjectionViewMatrices( const camera_t& cam ) { SetProjectionMatrix(cam); SetViewMatrix(cam); }
inline void SetViewport( uint x, uint y, uint w, uint h ) { glViewport(x,y,w,h); }

// deferred shading
namespace dfr {

extern void MaterialPass( const camera_t& cam );
extern void AmbientPass( const camera_t& cam, const vec3_t& color );
extern void LightingPass( const camera_t& cam );

} // end namespace dfr


} // end namespace
#endif
