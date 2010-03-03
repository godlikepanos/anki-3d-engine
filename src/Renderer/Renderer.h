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
namespace R { // begin namespace


extern uint  w; ///< width of the rendering window
extern uint  h; ///< height of the rendering window
//extern float renderingQuality; ///< The global rendering quality of the raster image. From 0.0(low) to 1.0(high)
extern uint  framesNum;
extern float aspectRatio;
extern int   screenshotJpegQuality; ///< The quality of the JPEG screenshots. From 0 to 100

extern int  maxColorAtachments; ///< Max color atachements a FBO can accept

// texture stuff
extern bool textureCompression; ///< Used in Texture::load to enable texture compression. Decreases video memory usage
extern int  maxTextureUnits; ///< Used in Texture::bind so we wont bind in a nonexistent texture unit. Readonly
extern bool mipmaping; ///< Used in Texture::load. Enables mipmaping increases video memory usage
extern int  max_anisotropy; ///< Max texture anisotropy. Used in Texture::load

// misc
extern void takeScreenshot( const char* filename ); ///< Save the colorbuffer as 24bit uncompressed TGA image
extern void init(); ///< Inits the renderer subsystem. Setting OpenGL and executes "R::*::init" functions among other things
extern void prepareNextFrame(); ///< Runs before rendering
extern const uchar* getLastError(); ///< getLastError
extern void printLastError(); ///< prints last OpenGL error
inline const string& getStdShaderPreprocDefines() { extern string std_shader_preproc_defines; return std_shader_preproc_defines; }
extern void render( const Camera& cam ); ///< The spine function of the renderer

extern float quad_vert_cords [][2];
extern void DrawQuad( int vertCoords_uni_loc );

// matrices
//extern

// ogl and glu wrappers
inline void   multMatrix( const Mat4& m4 ) { glMultMatrixf( &(m4.getTransposed())(0,0) ); } ///< OpenGL wrapper
inline void   loadMatrix( const Mat4& m4 ) { glLoadMatrixf( &(m4.getTransposed())(0,0) ); } ///< OpenGL wrapper

inline void   color3( const Vec3& v ) { glColor3fv( &((Vec3&)v)[0] ); } ///< OpenGL wrapper
inline void   color4( const Vec4& v ) { glColor4fv( &((Vec4&)v)[0] ); } ///< OpenGL wrapper
inline void   noShaders() { ShaderProg::unbind(); } ///< unbind shaders
extern bool   unproject( float winX, float winY, float winZ, const Mat4& modelview_mat, const Mat4& projection_mat, const int* view, float& objX, float& objY, float& objZ ); ///< My version of gluUnproject
extern Mat4   ortho( float left, float right, float bottom, float top, float near, float far );

// Matrix stuff
extern void setProjectionMatrix( const Camera& cam );
extern void setViewMatrix( const Camera& cam );
inline void setProjectionViewMatrices( const Camera& cam ) { setProjectionMatrix(cam); setViewMatrix(cam); }
inline void setViewport( uint x, uint y, uint w, uint h ) { glViewport(x,y,w,h); }

// externals that have global scope in other namespaces

/// material stage namespace
namespace Ms
{
	extern Texture normalFai, diffuseFai, specularFai, depthFai;

	extern void init();
	extern void runStage( const Camera& cam );

#if defined( _EARLY_Z_ )
	/// EarlyZ depth pass namespace
	namespace earlyz
	{
		extern void init(); ///< Inits the the earlyz FBO with R::Ms::depthFai
		extern void runPass( const Camera& cam ); ///< Renders the Scene's depth in the R::ms:depthFai
	}
#endif
}

/// illumination stage namespace
namespace Is
{
	extern Texture fai;

	extern void init();
	extern void runStage( const Camera& cam );

	/// illumination stage shadows namesapce
	namespace Shad
	{
		extern void init();
		extern void runPass( const Camera& cam );
		extern Texture shadowMap;
		extern int shadowResolution;
		extern bool pcf; ///< PCF on/off
		extern bool bilinear; ///< Shadowmap bilinear filtering on/off
	}
}

/// blending stage namespace
namespace Bs
{
	extern Texture faiBsScene;

	extern void init2();
	extern void runStage2( const Camera& cam );
	extern void init();
	extern void runStage( const Camera& cam );
}

/// pre-processing stage namespace
namespace Pps
{
	extern void init();
	extern void runStage( const Camera& cam );

	namespace Ssao
	{
		extern bool enabled;
		extern void init();
		extern void runPass( const Camera& cam );
		extern Texture pass0Fai;
		extern Texture pass1Fai;
		extern Texture fai;
		extern float renderingQuality;
	}

	namespace edgeaa
	{
		extern bool enabled;
	}

	namespace Hdr
	{
		extern bool    enabled;
		extern void    init();
		extern void    runPass( const Camera& cam );
		extern Texture pass0Fai;
		extern Texture pass1Fai;
		extern Texture fai;
		extern float   renderingQuality;
	}

	namespace Lscatt
	{
		extern bool enabled;
		extern void init();
		extern void runPass( const Camera& cam );
		extern Texture fai;
		extern float renderingQuality;
	}

	extern Texture fai;
}

/// debug stage namespace
namespace Dbg
{
	extern bool showAxis;
	extern bool showFnormals;
	extern bool showVnormals;
	extern bool showLights;
	extern bool showSkeletons;
	extern bool showCameras;
	extern bool showBvolumes;

	extern void init();
	extern void runStage( const Camera& cam );

	extern void renderGrid();
	extern void renderSphere( float radius, int precision );
	extern void renderCube( bool cols = false, float size = 1.0f );
	extern void renderQuad( float w, float h );
}

} // end namespace
#endif
