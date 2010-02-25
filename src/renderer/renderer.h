#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "Common.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Math.h"
#include "ShaderProg.h"
#include "Camera.h"



class Camera;

/// renderer namespace
namespace r { // begin namespace


extern uint  w; ///< width of the rendering window
extern uint  h; ///< height of the rendering window
//extern float rendering_quality; ///< The global rendering quality of the raster image. From 0.0(low) to 1.0(high)
extern uint  frames_num;
extern float aspect_ratio;
extern int   screenshot_jpeg_quality; ///< The quality of the JPEG screenshots. From 0 to 100

extern int  max_color_atachments; ///< Max color atachements a FBO can accept

// texture stuff
extern bool texture_compression; ///< Used in Texture::load to enable texture compression. Decreases video memory usage
extern int  max_texture_units; ///< Used in Texture::bind so we wont bind in a nonexistent texture unit. Readonly
extern bool mipmaping; ///< Used in Texture::load. Enables mipmaping increases video memory usage
extern int  max_anisotropy; ///< Max texture anisotropy. Used in Texture::load

// misc
extern void TakeScreenshot( const char* filename ); ///< Save the colorbuffer as 24bit uncompressed TGA image
extern void Init(); ///< Inits the renderer subsystem. Setting OpenGL and executes "r::*::init" functions among other things
extern void PrepareNextFrame(); ///< Runs before rendering
extern const uchar* GetLastError(); ///< GetLastError
extern void printLastError(); ///< prints last OpenGL error
inline const string& GetStdShaderPreprocDefines() { extern string std_shader_preproc_defines; return std_shader_preproc_defines; }
extern void Render( const Camera& cam ); ///< The spine function of the renderer

extern void SetGLState_Wireframe();
extern void SetGLState_Wireframedotted();
extern void SetGLState_Solid();
extern void SetGLState_AlphaSolid();

// matrices
//extern

// ogl and glu wrappers
inline void   MultMatrix( const Mat4& m4 ) { glMultMatrixf( &(m4.getTransposed())(0,0) ); } ///< OpenGL wrapper
inline void   loadMatrix( const Mat4& m4 ) { glLoadMatrixf( &(m4.getTransposed())(0,0) ); } ///< OpenGL wrapper

inline void   Color3( const Vec3& v ) { glColor3fv( &((Vec3&)v)[0] ); } ///< OpenGL wrapper
inline void   Color4( const Vec4& v ) { glColor4fv( &((Vec4&)v)[0] ); } ///< OpenGL wrapper
inline void   NoShaders() { ShaderProg::unbind(); } ///< unbind shaders
extern bool   Unproject( float winX, float winY, float winZ, const Mat4& modelview_mat, const Mat4& projection_mat, const int* view, float& objX, float& objY, float& objZ ); ///< My version of gluUnproject
extern Mat4 Ortho( float left, float right, float bottom, float top, float near, float far );

// Matrix stuff
extern void SetProjectionMatrix( const Camera& cam );
extern void SetViewMatrix( const Camera& cam );
inline void SetProjectionViewMatrices( const Camera& cam ) { SetProjectionMatrix(cam); SetViewMatrix(cam); }
inline void SetViewport( uint x, uint y, uint w, uint h ) { glViewport(x,y,w,h); }

// externals that have global scope in other namespaces

/// material stage namespace
namespace ms
{
	extern Texture normal_fai;
	extern Texture depth_fai;
}

/// illumination stage namespace
namespace is
{
	extern Texture fai;

	/// illumination stage shadows namesapce
	namespace shadows
	{
		extern ShaderProg* shdr_depth;
		extern ShaderProg* shdr_depth_grass;
		extern ShaderProg* shdr_depth_hw_skinning;
		extern bool pcf; ///< PCF on/off
		extern bool bilinear; ///< Shadowmap bilinear filtering on/off
	}
}

/// blending stage namespace
namespace bs
{
	extern void Init2();
	extern void RunStage2( const Camera& cam );
}

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

	extern Texture fai;
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
