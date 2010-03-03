/*
The file contains functions and vars used for the deferred shading illumination stage.
*/

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "Light.h"
#include "Resource.h"
#include "Scene.h"
#include "Fbo.h"
#include "LightProps.h"

namespace R {
namespace Is {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static Fbo fbo;

Texture fai;  // illuminated Scene

static uint stencilRb; // framebuffer render buffer for stencil optimizations

// shaders
class AmbientShaderProg: public ShaderProg
{
	public:
		struct
		{
			int ambientCol;
			int sceneColMap;
		}uniLocs;
};

static AmbientShaderProg ambientSProg;

class LightShaderProg: public ShaderProg
{
	public:
		struct
		{
			int msNormalFai;
			int msDiffuseFai;
			int msSpecularFai;
			int msDepthFai;
			int planes;
			int lightPos;
			int lightInvRadius;
			int lightDiffuseCol;
			int lightSpecularCol;
			int lightTex;
			int texProjectionMat;
			int shadowMap;
		}uniLocs;
};

static LightShaderProg pointLightSProg;
static LightShaderProg spotLightNoShadowSProg;
static LightShaderProg spotLightShadowSProg;


// the bellow are used to speedup the calculation of the frag pos (view space) inside the shader. This is done by precompute the
// view vectors each for one corner of the screen and the planes used to compute the frag_pos_view_space.z from the depth value.
static Vec3 viewVectors[4];
static Vec2 planes;


//=====================================================================================================================================
// Stencil Masking Opt Uv Sphere                                                                                                      =
//=====================================================================================================================================
static float smo_uvs_coords [] = { -0.000000, 0.000000, -1.000000, 0.500000, 0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.707107, 0.000000, 0.707107, -0.000000, 0.707107, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.000000, 0.707107, -0.707107, 0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.000000, 0.707107, 0.707107, -0.707107, -0.000000, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.707107, -0.000000, -0.707107, -0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, -0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.707107, -0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, -0.500000, -0.500000, -0.707107, -0.000000, 0.000000, -1.000000, 0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.707107, 0.000000, -0.707107, 0.500000, -0.500000, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 1.000000, 0.000000, -0.000000, 0.500000, -0.500000, -0.707107, 1.000000, 0.000000, -0.000000, 0.707107, -0.707107, 0.000000, 0.707107, -0.707107, 0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.000000, 0.707107, 0.707107, -0.707107, 0.000000, 0.707107, 0.000000, 0.707107, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.707107, -0.707107, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, -0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, -0.707107, 0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, 0.707107, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.500000, -0.500000, 0.707107, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, -0.707107, -0.707107, -0.707107, 0.000000, -0.707107, -0.000000, -0.707107, -0.707107, -0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -1.000000, -0.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.707107, -0.000000, 0.707107, -0.707107, 0.707107, 0.000000, -0.707107, -0.000000, 0.707107, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.707107, 0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 0.707107, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 1.000000, 0.000000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.707107, 0.707107, 0.000000, -0.000000, 0.707107, 0.707107, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.000000, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.500000, 0.500000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 1.000000, 0.000000, -0.000000 };
static uint smo_uvs_vbo_id = 0; // stencil masking opt uv sphere vertex buffer object id

// init stencil masking optimization UV sphere
static void initSMOUVS()
{
	glGenBuffers( 1, &smo_uvs_vbo_id );
	glBindBuffer( GL_ARRAY_BUFFER, smo_uvs_vbo_id );
	glBufferData( GL_ARRAY_BUFFER, sizeof(smo_uvs_coords), smo_uvs_coords, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

static void DrawSMOUVS( const PointLight& light )
{
	const float scale = 1.2;
	R::multMatrix( Mat4( light.translationWspace, Mat3::getIdentity(), light.radius*scale ) );

	R::noShaders();

	glBindBuffer( GL_ARRAY_BUFFER, smo_uvs_vbo_id );
	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDrawArrays( GL_TRIANGLES, 0, sizeof(smo_uvs_coords)/sizeof(float)/3 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


//=====================================================================================================================================
// CalcViewVector                                                                                                                     =
//=====================================================================================================================================
/// Calc the view vector that we will use inside the shader to calculate the frag pos in view space
static void CalcViewVector( const Camera& cam )
{
	int _w = R::w;
	int _h = R::h;
	int pixels[4][2]={ {_w,_h}, {0,_h}, { 0,0 }, {_w,0} }; // from righ up and CC wise to right down, Just like we render the quad
	int viewport[4]={ 0, 0, _w, _h };

	for( int i=0; i<4; i++ )
	{
		/* Original Code:
		R::UnProject( pixels[i][0], pixels[i][1], 10, cam.getViewMatrix(), cam.getProjectionMatrix(), viewport,
		              view_vectors[i].x, view_vectors[i].y, view_vectors[i].z );
		view_vectors[i] = cam.getViewMatrix() * view_vectors[i];
		The original code is the above 3 lines. The optimized follows:*/

		Vec3 vec;
		vec.x = (2.0*(pixels[i][0]-viewport[0]))/viewport[2] - 1.0;
		vec.y = (2.0*(pixels[i][1]-viewport[1]))/viewport[3] - 1.0;
		vec.z = 1.0;

		viewVectors[i] = vec.getTransformed( cam.getInvProjectionMatrix() );
		// end of optimized code
	}
}


//=====================================================================================================================================
// CalcPlanes                                                                                                                         =
//=====================================================================================================================================
/// Calc the planes that we will use inside the shader to calculate the frag pos in view space
static void CalcPlanes( const Camera& cam )
{
	planes.x = -cam.getZFar() / (cam.getZFar() - cam.getZNear());
	planes.y = -cam.getZFar() * cam.getZNear() / (cam.getZFar() - cam.getZNear());
}


//=====================================================================================================================================
// initStageFbo                                                                                                                       =
//=====================================================================================================================================
static void initStageFbo()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// init the stencil render buffer
	glGenRenderbuffers( 1, &stencilRb );
	glBindRenderbuffer( GL_RENDERBUFFER, stencilRb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX, R::w, R::h );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRb );

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// create the txtrs
	if( !fai.createEmpty2D( R::w, R::h, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE ) )
	{
		FATAL( "See prev error" );
	}

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create deferred shading illumination stage FBO" );

	// unbind
	fbo.unbind();
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void init()
{
	// load the shaders
	ambientSProg.customLoad( "shaders/is_ap.glsl" );
	ambientSProg.uniLocs.ambientCol = ambientSProg.getUniVar("ambientCol").getLoc();
	ambientSProg.uniLocs.sceneColMap = ambientSProg.getUniVar("sceneColMap").getLoc();

	pointLightSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _POINT_LIGHT_\n" );
	pointLightSProg.uniLocs.msNormalFai = pointLightSProg.getUniVar("msNormalFai").getLoc();
	pointLightSProg.uniLocs.msDiffuseFai = pointLightSProg.getUniVar("msDiffuseFai").getLoc();
	pointLightSProg.uniLocs.msSpecularFai = pointLightSProg.getUniVar("msSpecularFai").getLoc();
	pointLightSProg.uniLocs.msDepthFai = pointLightSProg.getUniVar("msDepthFai").getLoc();
	pointLightSProg.uniLocs.planes = pointLightSProg.getUniVar("planes").getLoc();
	pointLightSProg.uniLocs.lightPos = pointLightSProg.getUniVar("lightPos").getLoc();
	pointLightSProg.uniLocs.lightInvRadius = pointLightSProg.getUniVar("lightInvRadius").getLoc();
	pointLightSProg.uniLocs.lightDiffuseCol = pointLightSProg.getUniVar("lightDiffuseCol").getLoc();
	pointLightSProg.uniLocs.lightSpecularCol = pointLightSProg.getUniVar("lightSpecularCol").getLoc();

	spotLightNoShadowSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _SPOT_LIGHT_\n" );
	spotLightNoShadowSProg.uniLocs.msNormalFai = spotLightNoShadowSProg.getUniVar("msNormalFai").getLoc();
	spotLightNoShadowSProg.uniLocs.msDiffuseFai = spotLightNoShadowSProg.getUniVar("msDiffuseFai").getLoc();
	spotLightNoShadowSProg.uniLocs.msSpecularFai = spotLightNoShadowSProg.getUniVar("msSpecularFai").getLoc();
	spotLightNoShadowSProg.uniLocs.msDepthFai = spotLightNoShadowSProg.getUniVar("msDepthFai").getLoc();
	spotLightNoShadowSProg.uniLocs.planes = spotLightNoShadowSProg.getUniVar("planes").getLoc();
	spotLightNoShadowSProg.uniLocs.lightPos = spotLightNoShadowSProg.getUniVar("lightPos").getLoc();
	spotLightNoShadowSProg.uniLocs.lightInvRadius = spotLightNoShadowSProg.getUniVar("lightInvRadius").getLoc();
	spotLightNoShadowSProg.uniLocs.lightDiffuseCol = spotLightNoShadowSProg.getUniVar("lightDiffuseCol").getLoc();
	spotLightNoShadowSProg.uniLocs.lightSpecularCol = spotLightNoShadowSProg.getUniVar("lightSpecularCol").getLoc();
	spotLightNoShadowSProg.uniLocs.lightTex = spotLightNoShadowSProg.getUniVar("lightTex").getLoc();
	spotLightNoShadowSProg.uniLocs.texProjectionMat = spotLightNoShadowSProg.getUniVar("texProjectionMat").getLoc();

	spotLightShadowSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _SPOT_LIGHT_\n#define _SHADOW_\n" );
	spotLightShadowSProg.uniLocs.msNormalFai = spotLightShadowSProg.getUniVar("msNormalFai").getLoc();
	spotLightShadowSProg.uniLocs.msDiffuseFai = spotLightShadowSProg.getUniVar("msDiffuseFai").getLoc();
	spotLightShadowSProg.uniLocs.msSpecularFai = spotLightShadowSProg.getUniVar("msSpecularFai").getLoc();
	spotLightShadowSProg.uniLocs.msDepthFai = spotLightShadowSProg.getUniVar("msDepthFai").getLoc();
	spotLightShadowSProg.uniLocs.planes = spotLightShadowSProg.getUniVar("planes").getLoc();
	spotLightShadowSProg.uniLocs.lightPos = spotLightShadowSProg.getUniVar("lightPos").getLoc();
	spotLightShadowSProg.uniLocs.lightInvRadius = spotLightShadowSProg.getUniVar("lightInvRadius").getLoc();
	spotLightShadowSProg.uniLocs.lightDiffuseCol = spotLightShadowSProg.getUniVar("lightDiffuseCol").getLoc();
	spotLightShadowSProg.uniLocs.lightSpecularCol = spotLightShadowSProg.getUniVar("lightSpecularCol").getLoc();
	spotLightShadowSProg.uniLocs.lightTex = spotLightShadowSProg.getUniVar("lightTex").getLoc();
	spotLightShadowSProg.uniLocs.texProjectionMat = spotLightShadowSProg.getUniVar("texProjectionMat").getLoc();
	spotLightShadowSProg.uniLocs.shadowMap = spotLightShadowSProg.getUniVar("shadowMap").getLoc();


	// init the rest
	initStageFbo();
	initSMOUVS();

	R::Is::Shad::init();
}


//=====================================================================================================================================
// ambientPass                                                                                                                        =
//=====================================================================================================================================
static void ambientPass( const Camera& /*cam*/, const Vec3& color )
{
	glDisable( GL_BLEND );

	// set the shader
	ambientSProg.bind();

	// set the uniforms
	glUniform3fv( ambientSProg.uniLocs.ambientCol, 1, &(const_cast<Vec3&>(color)[0]) );
	ambientSProg.locTexUnit( ambientSProg.uniLocs.sceneColMap, R::Ms::diffuseFai, 0 );

	// Draw quad
	R::DrawQuad( 0 );
}


//=====================================================================================================================================
// setStencilMask [point light]                                                                                                       =
//=====================================================================================================================================
/// Clears the stencil buffer and draws a shape in the stencil buffer (in this case the shape is a UV shpere)
static void setStencilMask( const Camera& cam, const PointLight& light )
{
	glEnable( GL_STENCIL_TEST );
	glClear( GL_STENCIL_BUFFER_BIT );

	glColorMask( false, false, false, false );
	glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	glDisable( GL_CULL_FACE );

	// set matrices
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	R::setProjectionViewMatrices( cam );


	// render sphere to stencil buffer
	DrawSMOUVS( light );


	// restore matrices
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();


	glEnable( GL_CULL_FACE );
	glColorMask( true, true, true, true );

	// change the stencil func so that the light pass will only write in the masked area
	glStencilFunc( GL_EQUAL, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
}


//=====================================================================================================================================
// setStencilMask                                                                                                                     =
//=====================================================================================================================================
static void setStencilMask( const Camera& cam, const SpotLight& light )
{
	glEnable( GL_STENCIL_TEST );
	glClear( GL_STENCIL_BUFFER_BIT );

	glColorMask( false, false, false, false );
	glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	glDisable( GL_CULL_FACE );

	// set matrices
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	R::setProjectionViewMatrices( cam );


	// render camera's shape to stencil buffer
	R::noShaders();
	const Camera& lcam = light.camera;
	float x = lcam.getZFar() / tan( (PI-lcam.getFovX())/2 );
	float y = tan( lcam.getFovY()/2 ) * lcam.getZFar();
	float z = -lcam.getZFar();

	const int tris_num = 6;

	float verts[tris_num][3][3] = {
		{ { 0.0, 0.0, 0.0 }, { x, -y, z }, { x,  y, z } }, // right triangle
		{ { 0.0, 0.0, 0.0 }, { x,  y, z }, {-x,  y, z } }, // top
		{ { 0.0, 0.0, 0.0 }, {-x,  y, z }, {-x, -y, z } }, // left
		{ { 0.0, 0.0, 0.0 }, {-x, -y, z }, { x, -y, z } }, // bottom
		{ { x, -y, z }, {-x,  y, z }, { x,  y, z } }, // front up right
		{ { x, -y, z }, {-x, -y, z }, {-x,  y, z } }, // front bottom left
	};

	R::multMatrix( lcam.transformationWspace );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, verts );
	glDrawArrays( GL_TRIANGLES, 0, tris_num*3 );
	glDisableClientState( GL_VERTEX_ARRAY );


	// restore matrices
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();


	glEnable( GL_CULL_FACE );
	glColorMask( true, true, true, true );

	// change the stencil func so that the light pass will only write in the masked area
	glStencilFunc( GL_EQUAL, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
}


/*
=======================================================================================================================================
PointLightPass                                                                                                                        =
=======================================================================================================================================
*/
static void PointLightPass( const Camera& cam, const PointLight& light )
{
	//** make a check wether the point light passes the frustum test **
	bsphere_t sphere( light.translationWspace, light.radius );
	if( !cam.insideFrustum( sphere ) ) return;

	//** set the scissors **
	//int n = SetScissors( cam.getViewMatrix()*light.translationWspace, light.radius );
	//if( n < 1 ) return;

	//** stencil optimization **
	setStencilMask( cam, light );

	//** bind the shader **
	const LightShaderProg& shader = pointLightSProg; // I dont want to type
	shader.bind();

	// bind the material stage framebuffer attachable images
	shader.locTexUnit( shader.uniLocs.msDepthFai, R::Ms::normalFai, 0 );
	shader.locTexUnit( shader.uniLocs.msDiffuseFai, R::Ms::diffuseFai, 1 );
	shader.locTexUnit( shader.uniLocs.msSpecularFai, R::Ms::specularFai, 2 );
	shader.locTexUnit( shader.uniLocs.msDepthFai, R::Ms::depthFai, 3 );
	glUniform2fv( shader.uniLocs.planes, 1, &planes[0] );

	Vec3 lightPosEyeSpace = light.translationWspace.getTransformed( cam.getViewMatrix() );
	glUniform3fv( shader.uniLocs.lightPos, 1, &lightPosEyeSpace[0] );
	glUniform1f( shader.uniLocs.lightInvRadius, 1.0/light.radius );
	glUniform3fv( shader.uniLocs.lightDiffuseCol, 1, &Vec3(light.lightProps->getDiffuseColor())[0] );
	glUniform3fv( shader.uniLocs.lightSpecularCol, 1, &Vec3(light.lightProps->getSpecularColor())[0] );

	//** render quad **
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 0, 2, GL_FLOAT, false, 0, &R::quad_vert_cords[0] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, false, 0, &viewVectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );

	//glDisable( GL_SCISSOR_TEST );
	glDisable( GL_STENCIL_TEST );
}


/*
=======================================================================================================================================
SpotLightPass                                                                                                                         =
=======================================================================================================================================
*/
static void SpotLightPass( const Camera& cam, const SpotLight& light )
{
	//** first of all check if the light's camera is inside the frustum **
	if( !cam.insideFrustum( light.camera ) ) return;

	//** stencil optimization **
	setStencilMask( cam, light );

	//** generate the shadow map (if needed) **
	if( light.castsShadow )
	{
		R::Is::Shad::runPass( light.camera );

		// restore the IS FBO
		fbo.bind();

		// and restore blending and depth test
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
		glDisable( GL_DEPTH_TEST );
	}

	//** set the shader and uniforms **
	const LightShaderProg* shdr; // because of the huge name

	if( light.castsShadow )  shdr = &spotLightShadowSProg;
	else                     shdr = &spotLightNoShadowSProg;

	shdr->bind();

	// bind the framebuffer attachable images
	shdr->locTexUnit( shdr->uniLocs.msNormalFai, R::Ms::normalFai, 0 );
	shdr->locTexUnit( shdr->uniLocs.msDiffuseFai, R::Ms::diffuseFai, 1 );
	shdr->locTexUnit( shdr->uniLocs.msSpecularFai, R::Ms::specularFai, 2 );
	shdr->locTexUnit( shdr->uniLocs.msDepthFai, R::Ms::depthFai, 3 );

	if( light.lightProps->getTexture() == NULL )
		ERROR( "No texture is attached to the light. light_props name: " << light.lightProps->getRsrcName() );

	// the planes
	//glUniform2fv( shdr->getUniLoc("planes"), 1, &planes[0] );
	glUniform2fv( shdr->uniLocs.planes, 1, &planes[0] );

	// the light params
	Vec3 light_pos_eye_space = light.translationWspace.getTransformed( cam.getViewMatrix() );
	glUniform3fv( shdr->uniLocs.lightPos, 1, &light_pos_eye_space[0] );
	glUniform1f( shdr->uniLocs.lightInvRadius, 1.0/light.getDistance() );
	glUniform3fv( shdr->uniLocs.lightDiffuseCol, 1, &Vec3(light.lightProps->getDiffuseColor())[0] );
	glUniform3fv( shdr->uniLocs.lightSpecularCol, 1, &Vec3(light.lightProps->getSpecularColor())[0] );

	// set the light texture
	shdr->locTexUnit( shdr->uniLocs.lightTex, *light.lightProps->getTexture(), 4 );
	// before we render disable anisotropic in the light.texture because it produces artefacts. ToDo: see if this is unececeary in future drivers
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


	//** set texture matrix for shadowmap projection **
	// Bias * P_light * V_light * inv( V_cam )
	static Mat4 bias_m4( 0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0 );
	Mat4 tex_projection_mat;
	tex_projection_mat = bias_m4 * light.camera.getProjectionMatrix() * light.camera.getViewMatrix() * cam.transformationWspace;
	glUniformMatrix4fv( shdr->uniLocs.texProjectionMat, 1, true, &tex_projection_mat[0] );

	// the shadow stuff
	// render depth to texture and then bind it
	if( light.castsShadow )
	{
		shdr->locTexUnit( shdr->uniLocs.shadowMap, R::Is::Shad::shadowMap, 5 );
	}

	//** render quad **
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 0, 2, GL_FLOAT, false, 0, &R::quad_vert_cords[0] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, false, 0, &viewVectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );

	// restore texture matrix
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable( GL_STENCIL_TEST );
}


/*
=======================================================================================================================================
runStage                                                                                                                              =
=======================================================================================================================================
*/
void runStage( const Camera& cam )
{
	// FBO
	fbo.bind();

	// OGL stuff
	R::setViewport( 0, 0, R::w, R::h );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );

	// ambient pass
	ambientPass( cam, Scene::getAmbientColor() );

	// light passes
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	CalcViewVector( cam );
	CalcPlanes( cam );

	// for all lights
	for( uint i=0; i<Scene::lights.size(); i++ )
	{
		const Light& light = *Scene::lights[i];
		switch( light.type )
		{
			case Light::LT_POINT:
			{
				const PointLight& pointl = static_cast<const PointLight&>(light);
				PointLightPass( cam, pointl );
				break;
			}

			case Light::LT_SPOT:
			{
				const SpotLight& projl = static_cast<const SpotLight&>(light);
				SpotLightPass( cam, projl );
				break;
			}

			default:
				DEBUG_ERR( 1 );
		}
	}

	// FBO
	fbo.unbind();
}

}} // end namespaces
