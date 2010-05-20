#include "Renderer.hpp"
#include "Camera.h"
#include "Light.h"
#include "LightProps.h"
#include "App.h"
#include "Scene.h"


//=====================================================================================================================================
// STATICS                                                                                                                            =
//=====================================================================================================================================
float Renderer::Is::sMOUvSCoords[] = { -0.000000, 0.000000, -1.000000, 0.500000, 0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.707107, 0.000000, 0.707107, -0.000000, 0.707107, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.000000, 0.707107, -0.707107, 0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.000000, 0.707107, 0.707107, -0.707107, -0.000000, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.707107, -0.000000, -0.707107, -0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, -0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.707107, -0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, -0.500000, -0.500000, -0.707107, -0.000000, 0.000000, -1.000000, 0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.707107, 0.000000, -0.707107, 0.500000, -0.500000, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 1.000000, 0.000000, -0.000000, 0.500000, -0.500000, -0.707107, 1.000000, 0.000000, -0.000000, 0.707107, -0.707107, 0.000000, 0.707107, -0.707107, 0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.000000, 0.707107, 0.707107, -0.707107, 0.000000, 0.707107, 0.000000, 0.707107, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.707107, -0.707107, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, -0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, -0.707107, 0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, 0.707107, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.500000, -0.500000, 0.707107, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, -0.707107, -0.707107, -0.707107, 0.000000, -0.707107, -0.000000, -0.707107, -0.707107, -0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -1.000000, -0.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.707107, -0.000000, 0.707107, -0.707107, 0.707107, 0.000000, -0.707107, -0.000000, 0.707107, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.707107, 0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 0.707107, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 1.000000, 0.000000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.707107, 0.707107, 0.000000, -0.000000, 0.707107, 0.707107, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.000000, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.500000, 0.500000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 1.000000, 0.000000, -0.000000 };
uint Renderer::Is::sMOUvSVboId = 0;


//=====================================================================================================================================
// initSMOUvS                                                                                                                         =
//=====================================================================================================================================
void Renderer::Is::initSMOUvS()
{
	DEBUG_ERR( sMOUvSVboId != 0 );
	glGenBuffers( 1, &sMOUvSVboId );
	glBindBuffer( GL_ARRAY_BUFFER, sMOUvSVboId );
	glBufferData( GL_ARRAY_BUFFER, sizeof(sMOUvSVboId), sMOUvSCoords, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


//=====================================================================================================================================
// renderSMOUvS                                                                                                                       =
//=====================================================================================================================================
void Renderer::Is::renderSMOUvS( const PointLight& light )
{
	const float scale = 1.2;

	/// @todo Replace the rendering code. Use shader prog

	/// @todo correct the bellow
	/*R::multMatrix( Mat4( light.getWorldTransform().getOrigin(), Mat3::getIdentity(), light.radius*scale ) );
	R::noShaders();*/

	glBindBuffer( GL_ARRAY_BUFFER, sMOUvSVboId );
	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDrawArrays( GL_TRIANGLES, 0, sizeof(sMOUvSCoords)/sizeof(float)/3 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


//=====================================================================================================================================
// CalcViewVector                                                                                                                     =
//=====================================================================================================================================
void Renderer::Is::calcViewVector()
{
	const Camera& cam = *r.cam;

	int w = r.width;
	int h = r.height;
	int pixels[4][2]={ {w,h}, {0,h}, { 0,0 }, {w,0} }; // from righ up and CC wise to right down, Just like we render the quad
	int viewport[4]={ 0, 0, w, h };

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
// calcPlanes                                                                                                                         =
//=====================================================================================================================================
void Renderer::Is::calcPlanes()
{
	const Camera& cam = *r.cam;

	planes.x = -cam.getZFar() / (cam.getZFar() - cam.getZNear());
	planes.y = -cam.getZFar() * cam.getZNear() / (cam.getZFar() - cam.getZNear());
}


//=====================================================================================================================================
// initFbo                                                                                                                            =
//=====================================================================================================================================
void Renderer::Is::initFbo()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// init the stencil render buffer
	glGenRenderbuffers( 1, &stencilRb );
	glBindRenderbuffer( GL_RENDERBUFFER, stencilRb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX, r.width, r.height );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRb );

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements( 1 );

	// create the txtrs
	if( !fai.createEmpty2D( r.width, r.height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE ) )
		FATAL( "Cannot create deferred shading illumination stage FAI" );

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
void Renderer::Is::init()
{
	/// @todo see what to do with the commented code

	// load the shaders
	ambientPassSProg.customLoad( "shaders/is_ap.glsl" );
	ambientPassSProg.uniLocs.ambientCol = ambientPassSProg.getUniVar("ambientCol")->getLoc();
	ambientPassSProg.uniLocs.sceneColMap = ambientPassSProg.getUniVar("sceneColMap")->getLoc();

	pointLightSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _POINT_LIGHT_\n" );
	pointLightSProg.uniLocs.msNormalFai = pointLightSProg.getUniVar("msNormalFai")->getLoc();
	pointLightSProg.uniLocs.msDiffuseFai = pointLightSProg.getUniVar("msDiffuseFai")->getLoc();
	pointLightSProg.uniLocs.msSpecularFai = pointLightSProg.getUniVar("msSpecularFai")->getLoc();
	pointLightSProg.uniLocs.msDepthFai = pointLightSProg.getUniVar("msDepthFai")->getLoc();
	pointLightSProg.uniLocs.planes = pointLightSProg.getUniVar("planes")->getLoc();
	pointLightSProg.uniLocs.lightPos = pointLightSProg.getUniVar("lightPos")->getLoc();
	pointLightSProg.uniLocs.lightInvRadius = pointLightSProg.getUniVar("lightInvRadius")->getLoc();
	pointLightSProg.uniLocs.lightDiffuseCol = pointLightSProg.getUniVar("lightDiffuseCol")->getLoc();
	pointLightSProg.uniLocs.lightSpecularCol = pointLightSProg.getUniVar("lightSpecularCol")->getLoc();

	spotLightNoShadowSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _SPOT_LIGHT_\n" );
	spotLightNoShadowSProg.uniLocs.msNormalFai = spotLightNoShadowSProg.getUniVar("msNormalFai")->getLoc();
	spotLightNoShadowSProg.uniLocs.msDiffuseFai = spotLightNoShadowSProg.getUniVar("msDiffuseFai")->getLoc();
	spotLightNoShadowSProg.uniLocs.msSpecularFai = spotLightNoShadowSProg.getUniVar("msSpecularFai")->getLoc();
	spotLightNoShadowSProg.uniLocs.msDepthFai = spotLightNoShadowSProg.getUniVar("msDepthFai")->getLoc();
	spotLightNoShadowSProg.uniLocs.planes = spotLightNoShadowSProg.getUniVar("planes")->getLoc();
	spotLightNoShadowSProg.uniLocs.lightPos = spotLightNoShadowSProg.getUniVar("lightPos")->getLoc();
	spotLightNoShadowSProg.uniLocs.lightInvRadius = spotLightNoShadowSProg.getUniVar("lightInvRadius")->getLoc();
	spotLightNoShadowSProg.uniLocs.lightDiffuseCol = spotLightNoShadowSProg.getUniVar("lightDiffuseCol")->getLoc();
	spotLightNoShadowSProg.uniLocs.lightSpecularCol = spotLightNoShadowSProg.getUniVar("lightSpecularCol")->getLoc();
	spotLightNoShadowSProg.uniLocs.lightTex = spotLightNoShadowSProg.getUniVar("lightTex")->getLoc();
	spotLightNoShadowSProg.uniLocs.texProjectionMat = spotLightNoShadowSProg.getUniVar("texProjectionMat")->getLoc();

	spotLightShadowSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _SPOT_LIGHT_\n#define _SHADOW_\n" );
	spotLightShadowSProg.uniLocs.msNormalFai = spotLightShadowSProg.getUniVar("msNormalFai")->getLoc();
	spotLightShadowSProg.uniLocs.msDiffuseFai = spotLightShadowSProg.getUniVar("msDiffuseFai")->getLoc();
	spotLightShadowSProg.uniLocs.msSpecularFai = spotLightShadowSProg.getUniVar("msSpecularFai")->getLoc();
	spotLightShadowSProg.uniLocs.msDepthFai = spotLightShadowSProg.getUniVar("msDepthFai")->getLoc();
	spotLightShadowSProg.uniLocs.planes = spotLightShadowSProg.getUniVar("planes")->getLoc();
	spotLightShadowSProg.uniLocs.lightPos = spotLightShadowSProg.getUniVar("lightPos")->getLoc();
	spotLightShadowSProg.uniLocs.lightInvRadius = spotLightShadowSProg.getUniVar("lightInvRadius")->getLoc();
	spotLightShadowSProg.uniLocs.lightDiffuseCol = spotLightShadowSProg.getUniVar("lightDiffuseCol")->getLoc();
	spotLightShadowSProg.uniLocs.lightSpecularCol = spotLightShadowSProg.getUniVar("lightSpecularCol")->getLoc();
	spotLightShadowSProg.uniLocs.lightTex = spotLightShadowSProg.getUniVar("lightTex")->getLoc();
	spotLightShadowSProg.uniLocs.texProjectionMat = spotLightShadowSProg.getUniVar("texProjectionMat")->getLoc();
	spotLightShadowSProg.uniLocs.shadowMap = spotLightShadowSProg.getUniVar("shadowMap")->getLoc();


	// init the rest
	initFbo();
	if( sMOUvSVboId == 0 ) initSMOUvS();

	sm.init();
}


//=====================================================================================================================================
// ambientPass                                                                                                                        =
//=====================================================================================================================================
void Renderer::Is::ambientPass( const Vec3& color )
{
	glDisable( GL_BLEND );

	// set the shader
	ambientPassSProg.bind();

	// set the uniforms
	glUniform3fv( ambientPassSProg.uniLocs.ambientCol, 1, &(const_cast<Vec3&>(color)[0]) );
	ambientPassSProg.locTexUnit( ambientPassSProg.uniLocs.sceneColMap, r.ms.diffuseFai, 0 );

	// Draw quad
	r.drawQuad( 0 );
}


//=====================================================================================================================================
// setStencilMask [point light]                                                                                                       =
//=====================================================================================================================================
void Renderer::Is::setStencilMask( const PointLight& light )
{
	glEnable( GL_STENCIL_TEST );
	glClear( GL_STENCIL_BUFFER_BIT );

	glColorMask( false, false, false, false );
	glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	glDisable( GL_CULL_FACE );

	// set matrices
	/// @todo opegnl 3.x
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	r.setProjectionViewMatrices( *r.cam );

	// render sphere to stencil buffer
	renderSMOUvS( light );

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
// setStencilMask [spot light]                                                                                                        =
//=====================================================================================================================================
void Renderer::Is::setStencilMask( const SpotLight& light )
{
	glEnable( GL_STENCIL_TEST );
	glClear( GL_STENCIL_BUFFER_BIT );

	glColorMask( false, false, false, false );
	glStencilFunc( GL_ALWAYS, 0x1, 0x1 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	glDisable( GL_CULL_FACE );

	// set matrices
	/// @todo opengl 3.x
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	r.setProjectionViewMatrices( *r.cam );


	// render camera's shape to stencil buffer
	r.noShaders();
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

	r.multMatrix( Mat4(lcam.getWorldTransform()) );
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


//=====================================================================================================================================
// pointLightPass                                                                                                                     =
//=====================================================================================================================================
void Renderer::Is::pointLightPass( const PointLight& light )
{
	const Camera& cam = *r.cam;

	// make a check weather the point light passes the frustum test
	bsphere_t sphere( light.getWorldTransform().getOrigin(), light.radius );
	if( !cam.insideFrustum( sphere ) ) return;

	// stencil optimization
	setStencilMask( light );

	// bind the shader
	const LightShaderProg& shader = pointLightSProg; // I dont want to type
	shader.bind();

	// bind the material stage framebuffer attachable images
	shader.locTexUnit( shader.uniLocs.msDepthFai, r.ms.normalFai, 0 );
	shader.locTexUnit( shader.uniLocs.msDiffuseFai, r.ms.diffuseFai, 1 );
	shader.locTexUnit( shader.uniLocs.msSpecularFai, r.ms.specularFai, 2 );
	shader.locTexUnit( shader.uniLocs.msDepthFai, r.ms.depthFai, 3 );
	glUniform2fv( shader.uniLocs.planes, 1, &planes[0] );

	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed( cam.getViewMatrix() );
	glUniform3fv( shader.uniLocs.lightPos, 1, &lightPosEyeSpace[0] );
	glUniform1f( shader.uniLocs.lightInvRadius, 1.0/light.radius );
	glUniform3fv( shader.uniLocs.lightDiffuseCol, 1, &Vec3(light.lightProps->getDiffuseColor())[0] );
	glUniform3fv( shader.uniLocs.lightSpecularCol, 1, &Vec3(light.lightProps->getSpecularColor())[0] );

	// render quad
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 0, 2, GL_FLOAT, false, 0, &Renderer::quadVertCoords[0] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, false, 0, &viewVectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );

	// cleanup
	glDisable( GL_STENCIL_TEST );
}


//=====================================================================================================================================
// spotLightPass                                                                                                                      =
//=====================================================================================================================================
void Renderer::Is::spotLightPass( const SpotLight& light )
{
	const Camera& cam = *r.cam;

	//
	// first of all check if the light's camera is inside the frustum
	//
	if( !cam.insideFrustum( light.camera ) ) return;

	//
	// stencil optimization
	//
	setStencilMask( light );

	//
	// generate the shadow map (if needed)
	//
	if( light.castsShadow )
	{
		sm.run( light.camera );

		// restore the IS FBO
		fbo.bind();

		// and restore blending and depth test
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
		glDisable( GL_DEPTH_TEST );
	}

	//
	// set the shader and uniforms
	//
	const LightShaderProg* shdr; // because of the huge name

	if( light.castsShadow )  shdr = &spotLightShadowSProg;
	else                     shdr = &spotLightNoShadowSProg;

	shdr->bind();

	// bind the framebuffer attachable images
	shdr->locTexUnit( shdr->uniLocs.msNormalFai, r.ms.normalFai, 0 );
	shdr->locTexUnit( shdr->uniLocs.msDiffuseFai, r.ms.diffuseFai, 1 );
	shdr->locTexUnit( shdr->uniLocs.msSpecularFai, r.ms.specularFai, 2 );
	shdr->locTexUnit( shdr->uniLocs.msDepthFai, r.ms.depthFai, 3 );

	if( light.lightProps->getTexture() == NULL )
		ERROR( "No texture is attached to the light. lightProps name: " << light.lightProps->getRsrcName() );

	// the planes
	//glUniform2fv( shdr->getUniLoc("planes"), 1, &planes[0] );
	glUniform2fv( shdr->uniLocs.planes, 1, &planes[0] );

	// the light params
	Vec3 light_pos_eye_space = light.getWorldTransform().getOrigin().getTransformed( cam.getViewMatrix() );
	glUniform3fv( shdr->uniLocs.lightPos, 1, &light_pos_eye_space[0] );
	glUniform1f( shdr->uniLocs.lightInvRadius, 1.0/light.getDistance() );
	glUniform3fv( shdr->uniLocs.lightDiffuseCol, 1, &Vec3(light.lightProps->getDiffuseColor())[0] );
	glUniform3fv( shdr->uniLocs.lightSpecularCol, 1, &Vec3(light.lightProps->getSpecularColor())[0] );

	// set the light texture
	shdr->locTexUnit( shdr->uniLocs.lightTex, *light.lightProps->getTexture(), 4 );
	// before we render disable anisotropic in the light.texture because it produces artefacts. ToDo: see if this is unececeary in future drivers
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


	//
	// set texture matrix for shadowmap projection
	//
	// Bias * P_light * V_light * inv( V_cam )
	static Mat4 bias_m4( 0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0 );
	Mat4 texProjectionMat;
	texProjectionMat = bias_m4 * light.camera.getProjectionMatrix() * light.camera.getViewMatrix() * Mat4(cam.getWorldTransform());
	glUniformMatrix4fv( shdr->uniLocs.texProjectionMat, 1, true, &texProjectionMat[0] );

	// the shadow stuff
	// render depth to texture and then bind it
	if( light.castsShadow )
	{
		shdr->locTexUnit( shdr->uniLocs.shadowMap, sm.shadowMap, 5 );
	}

	//
	// render quad
	//
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 0, 2, GL_FLOAT, false, 0, &Renderer::quadVertCoords[0] );
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


//=====================================================================================================================================
// run                                                                                                                                =
//=====================================================================================================================================
void Renderer::Is::run()
{
	// FBO
	fbo.bind();

	// OGL stuff
	r.setViewport( 0, 0, r.width, r.height );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );

	// ambient pass
	ambientPass( app->getScene()->getAmbientCol() );

	// light passes
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	calcViewVector();
	calcPlanes();

	// for all lights
	for( uint i=0; i<app->getScene()->lights.size(); i++ )
	{
		const Light& light = *app->getScene()->lights[i];
		switch( light.type )
		{
			case Light::LT_POINT:
			{
				const PointLight& pointl = static_cast<const PointLight&>(light);
				pointLightPass( pointl );
				break;
			}

			case Light::LT_SPOT:
			{
				const SpotLight& projl = static_cast<const SpotLight&>(light);
				spotLightPass( projl );
				break;
			}

			default:
				DEBUG_ERR( 1 );
		}
	}

	// FBO
	fbo.unbind();
}
