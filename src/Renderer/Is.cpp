#include "Renderer.h"
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

	r.multMatrix( Mat4( light.getWorldTransform().getOrigin(), Mat3::getIdentity(), light.radius*scale ) );
	Renderer::noShaders();

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
	ambientPassSProg.uniVars.ambientCol = ambientPassSProg.findUniVar("ambientCol");
	ambientPassSProg.uniVars.sceneColMap = ambientPassSProg.findUniVar("sceneColMap");

	pointLightSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _POINT_LIGHT_\n" );
	pointLightSProg.uniVars.msNormalFai = pointLightSProg.findUniVar("msNormalFai");
	pointLightSProg.uniVars.msDiffuseFai = pointLightSProg.findUniVar("msDiffuseFai");
	pointLightSProg.uniVars.msSpecularFai = pointLightSProg.findUniVar("msSpecularFai");
	pointLightSProg.uniVars.msDepthFai = pointLightSProg.findUniVar("msDepthFai");
	pointLightSProg.uniVars.planes = pointLightSProg.findUniVar("planes");
	pointLightSProg.uniVars.lightPos = pointLightSProg.findUniVar("lightPos");
	pointLightSProg.uniVars.lightInvRadius = pointLightSProg.findUniVar("lightInvRadius");
	pointLightSProg.uniVars.lightDiffuseCol = pointLightSProg.findUniVar("lightDiffuseCol");
	pointLightSProg.uniVars.lightSpecularCol = pointLightSProg.findUniVar("lightSpecularCol");

	spotLightNoShadowSProg.customLoad( "shaders/is_lp_generic.glsl", "#define _SPOT_LIGHT_\n" );
	spotLightNoShadowSProg.uniVars.msNormalFai = spotLightNoShadowSProg.findUniVar("msNormalFai");
	spotLightNoShadowSProg.uniVars.msDiffuseFai = spotLightNoShadowSProg.findUniVar("msDiffuseFai");
	spotLightNoShadowSProg.uniVars.msSpecularFai = spotLightNoShadowSProg.findUniVar("msSpecularFai");
	spotLightNoShadowSProg.uniVars.msDepthFai = spotLightNoShadowSProg.findUniVar("msDepthFai");
	spotLightNoShadowSProg.uniVars.planes = spotLightNoShadowSProg.findUniVar("planes");
	spotLightNoShadowSProg.uniVars.lightPos = spotLightNoShadowSProg.findUniVar("lightPos");
	spotLightNoShadowSProg.uniVars.lightInvRadius = spotLightNoShadowSProg.findUniVar("lightInvRadius");
	spotLightNoShadowSProg.uniVars.lightDiffuseCol = spotLightNoShadowSProg.findUniVar("lightDiffuseCol");
	spotLightNoShadowSProg.uniVars.lightSpecularCol = spotLightNoShadowSProg.findUniVar("lightSpecularCol");
	spotLightNoShadowSProg.uniVars.lightTex = spotLightNoShadowSProg.findUniVar("lightTex");
	spotLightNoShadowSProg.uniVars.texProjectionMat = spotLightNoShadowSProg.findUniVar("texProjectionMat");

	string pps = "#define SHADOWMAP_SIZE " + Util::intToStr( sm.resolution ) + "\n#define _SPOT_LIGHT_\n#define _SHADOW_\n";
	spotLightShadowSProg.customLoad( "shaders/is_lp_generic.glsl", pps.c_str() );
	spotLightShadowSProg.uniVars.msNormalFai = spotLightShadowSProg.findUniVar("msNormalFai");
	spotLightShadowSProg.uniVars.msDiffuseFai = spotLightShadowSProg.findUniVar("msDiffuseFai");
	spotLightShadowSProg.uniVars.msSpecularFai = spotLightShadowSProg.findUniVar("msSpecularFai");
	spotLightShadowSProg.uniVars.msDepthFai = spotLightShadowSProg.findUniVar("msDepthFai");
	spotLightShadowSProg.uniVars.planes = spotLightShadowSProg.findUniVar("planes");
	spotLightShadowSProg.uniVars.lightPos = spotLightShadowSProg.findUniVar("lightPos");
	spotLightShadowSProg.uniVars.lightInvRadius = spotLightShadowSProg.findUniVar("lightInvRadius");
	spotLightShadowSProg.uniVars.lightDiffuseCol = spotLightShadowSProg.findUniVar("lightDiffuseCol");
	spotLightShadowSProg.uniVars.lightSpecularCol = spotLightShadowSProg.findUniVar("lightSpecularCol");
	spotLightShadowSProg.uniVars.lightTex = spotLightShadowSProg.findUniVar("lightTex");
	spotLightShadowSProg.uniVars.texProjectionMat = spotLightShadowSProg.findUniVar("texProjectionMat");
	spotLightShadowSProg.uniVars.shadowMap = spotLightShadowSProg.findUniVar("shadowMap");


	// init the rest
	initFbo();
	if( sMOUvSVboId == 0 )
		initSMOUvS();

	if( sm.enabled )
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
	ambientPassSProg.uniVars.ambientCol->setVec3( &color );
	ambientPassSProg.uniVars.sceneColMap->setTexture( r.ms.diffuseFai, 0 );

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
	shader.uniVars.msNormalFai->setTexture( r.ms.normalFai, 0 );
	shader.uniVars.msDiffuseFai->setTexture( r.ms.diffuseFai, 1 );
	shader.uniVars.msSpecularFai->setTexture( r.ms.specularFai, 2 );
	shader.uniVars.msDepthFai->setTexture( r.ms.depthFai, 3 );
	shader.uniVars.planes->setVec2( &planes );

	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed( cam.getViewMatrix() );
	shader.uniVars.lightPos->setVec3( &lightPosEyeSpace );
	shader.uniVars.lightInvRadius->setFloat( 1.0/light.radius );
	shader.uniVars.lightDiffuseCol->setVec3( &light.lightProps->getDiffuseColor() );
	shader.uniVars.lightSpecularCol->setVec3( &light.lightProps->getSpecularColor() );

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
	if( light.castsShadow && sm.enabled )
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

	if( light.castsShadow && sm.enabled )
		shdr = &spotLightShadowSProg;
	else
		shdr = &spotLightNoShadowSProg;

	shdr->bind();

	// bind the framebuffer attachable images
	shdr->uniVars.msNormalFai->setTexture( r.ms.normalFai, 0 );
	shdr->uniVars.msDiffuseFai->setTexture( r.ms.diffuseFai, 1 );
	shdr->uniVars.msSpecularFai->setTexture( r.ms.specularFai, 2 );
	shdr->uniVars.msDepthFai->setTexture( r.ms.depthFai, 3 );

	if( light.lightProps->getTexture() == NULL )
		ERROR( "No texture is attached to the light. lightProps name: " << light.lightProps->getRsrcName() );

	// the planes
	shdr->uniVars.planes->setVec2( &planes );

	// the light params
	Vec3 lightPosEyeSpace = light.getWorldTransform().getOrigin().getTransformed( cam.getViewMatrix() );
	shdr->uniVars.lightPos->setVec3( &lightPosEyeSpace );
	shdr->uniVars.lightInvRadius->setFloat( 1.0/light.getDistance() );
	shdr->uniVars.lightDiffuseCol->setVec3( &light.lightProps->getDiffuseColor() );
	shdr->uniVars.lightSpecularCol->setVec3( &light.lightProps->getSpecularColor() );

	// set the light texture
	shdr->uniVars.lightTex->setTexture( *light.lightProps->getTexture(), 3 );
	// before we render disable anisotropic in the light.texture because it produces artefacts. ToDo: see if this is unececeary in future drivers
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


	//
	// set texture matrix for texture & shadowmap projection
	//
	// Bias * P_light * V_light * inv( V_cam )
	static Mat4 biasMat4( 0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0 );
	Mat4 texProjectionMat;
	texProjectionMat = biasMat4 * light.camera.getProjectionMatrix() * light.camera.getViewMatrix() * Mat4(cam.getWorldTransform());
	shdr->uniVars.texProjectionMat->setMat4( &texProjectionMat );

	// the shadow stuff
	// render depth to texture and then bind it
	if( light.castsShadow && sm.enabled )
	{
		shdr->uniVars.shadowMap->setTexture( sm.shadowMap, 5 );
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
