#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "common.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "gmath.h"
#include "shader_prog.h"
#include "camera.h"



class camera_t;

/// renderer namespace
namespace r { // begin namespace


extern uint  w; ///< width of the rendering window
extern uint  h; ///< height of the rendering window
extern float rendering_quality; ///< The global rendering quality of the raster image. From 0.0(low) to 1.0(high)
extern uint  frames_num;
extern float aspect_ratio;
extern int   screenshot_jpeg_quality; ///< The quality of the JPEG screenshots. From 0 to 100

extern int  max_color_atachments; ///< Max color atachements a FBO can accept

// texture stuff
extern bool texture_compression; ///< Used in texture_t::Load to enable texture compression. Decreases video memory usage
extern int  max_texture_units; ///< Used in texture_t::Bind so we wont bind in a nonexistent texture unit. Readonly
extern bool mipmaping; ///< Used in texture_t::Load. Enables mipmaping increases video memory usage
extern int  max_anisotropy; ///< Max texture anisotropy. Used in texture_t::Load

// misc
extern void TakeScreenshot( const char* filename ); ///< Save the colorbuffer as 24bit uncompressed TGA image
extern void Init(); ///< Inits the renderer subsystem. Setting OpenGL and executes "r::*::Init" functions among other things
extern void PrepareNextFrame(); ///< Runs before rendering
extern const uchar* GetLastError(); ///< GetLastError
extern void PrintLastError(); ///< Prints last OpenGL error
inline const string& GetStdShaderPreprocDefines() { extern string std_shader_preproc_defines; return std_shader_preproc_defines; }
extern void Render( const camera_t& cam ); ///< The spine function of the renderer

extern void SetGLState_Wireframe();
extern void SetGLState_WireframeDotted();
extern void SetGLState_Solid();
extern void SetGLState_AlphaSolid();

// matrices
//extern

// ogl and glu wrappers
inline void   MultMatrix( const mat4_t& m4 ) { glMultMatrixf( &(m4.GetTransposed())(0,0) ); } ///< OpenGL wrapper
inline void   LoadMatrix( const mat4_t& m4 ) { glLoadMatrixf( &(m4.GetTransposed())(0,0) ); } ///< OpenGL wrapper

inline void   Color3( const vec3_t& v ) { glColor3fv( &((vec3_t&)v)[0] ); } ///< OpenGL wrapper
inline void   Color4( const vec4_t& v ) { glColor4fv( &((vec4_t&)v)[0] ); } ///< OpenGL wrapper
inline void   NoShaders() { shader_prog_t::Unbind(); } ///< Unbind shaders
extern bool   Unproject( float winX, float winY, float winZ, const mat4_t& modelview_mat, const mat4_t& projection_mat, const int* view, float& objX, float& objY, float& objZ ); ///< My version of gluUnproject
extern mat4_t Ortho( float left, float right, float bottom, float top, float near, float far );

// Matrix stuff
extern void SetProjectionMatrix( const camera_t& cam );
extern void SetViewMatrix( const camera_t& cam );
inline void SetProjectionViewMatrices( const camera_t& cam ) { SetProjectionMatrix(cam); SetViewMatrix(cam); }
inline void SetViewport( uint x, uint y, uint w, uint h ) { glViewport(x,y,w,h); }

// externals that have global scope in other namespaces

/// material stage namespace
namespace ms
{
	extern texture_t normal_fai;
	extern texture_t depth_fai;
}

/// illumination stage namespace
namespace is
{
	extern texture_t fai;

	/// illumination stage shadows namesapce
	namespace shadows
	{
		extern shader_prog_t* shdr_depth;
		extern shader_prog_t* shdr_depth_grass;
		extern shader_prog_t* shdr_depth_hw_skinning;
		extern bool pcf; ///< PCF on/off
		extern bool bilinear; ///< Shadowmap bilinear filtering on/off
	}
}

/// blending stage namespace
namespace bs
{}

/// pre-processing stage namespace
namespace pps
{
	namespace ssao
	{
		extern bool enabled; ///< SSAO on/of
	}
	namespace edgeaa
	{
		extern bool enabled; ///< EdgeAA on/of
	}
	namespace hdr
	{
		extern bool enabled; ///< Bloom on/of
	}
	namespace lscatt
	{
		extern bool enabled; ///< Light scattering on/of
	}

	extern texture_t fai;
}

/// debug stage namespace
namespace dbg
{
	extern bool show_axis;
	extern bool show_fnormals;
	extern bool show_vnormals;
	extern bool show_lights;
	extern bool show_skeletons;
	extern bool show_cameras;
	extern bool show_bvolumes;

	extern void RenderGrid();
	extern void RenderSphere( float radius, int precision );
	extern void RenderCube( bool cols = false, float size = 1.0f );
	extern void RenderQuad( float w, float h );
}

} // end namespace
#endif
