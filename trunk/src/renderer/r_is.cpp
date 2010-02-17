/*
The file contains functions and vars used for the deferred shading illumination stage.
*/

#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "light.h"
#include "resource.h"
#include "scene.h"
#include "r_private.h"
#include "fbo.h"
#include "light_props.h"

namespace r {
namespace is {


/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
static fbo_t fbo;

texture_t fai;  // illuminated scene

static uint stencil_rb; // framebuffer render buffer for stencil optimizations

// shaders
static shader_prog_t* shdr_is_ambient;
static shader_prog_t* shdr_is_lp_point_light;
static shader_prog_t* shdr_is_lp_spot_light_nos;
static shader_prog_t* shdr_is_lp_spot_light_s;


// the bellow are used to speedup the calculation of the frag pos (view space) inside the shader. This is done by precompute the
// view vectors each for one corner of the screen and tha planes used to compute the frag_pos_view_space.z from the depth value.
static vec3_t view_vectors[4];
static vec2_t planes;


/*
=======================================================================================================================================
Stencil Masking Opt Uv Sphere                                                                                                         =
=======================================================================================================================================
*/
static float smo_uvs_coords [] = { -0.000000, 0.000000, -1.000000, 0.500000, 0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.707107, 0.000000, 0.707107, -0.000000, 0.707107, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.000000, 0.707107, -0.707107, 0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.000000, 0.707107, 0.707107, -0.707107, -0.000000, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.707107, -0.000000, -0.707107, -0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, -0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.707107, -0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, -0.500000, -0.500000, -0.707107, -0.000000, 0.000000, -1.000000, 0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.707107, 0.000000, -0.707107, 0.500000, -0.500000, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 1.000000, 0.000000, -0.000000, 0.500000, -0.500000, -0.707107, 1.000000, 0.000000, -0.000000, 0.707107, -0.707107, 0.000000, 0.707107, -0.707107, 0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.000000, 0.707107, 0.707107, -0.707107, 0.000000, 0.707107, 0.000000, 0.707107, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.707107, -0.707107, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, -0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, -0.707107, 0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, 0.707107, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.500000, -0.500000, 0.707107, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, -0.707107, -0.707107, -0.707107, 0.000000, -0.707107, -0.000000, -0.707107, -0.707107, -0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -1.000000, -0.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.707107, -0.000000, 0.707107, -0.707107, 0.707107, 0.000000, -0.707107, -0.000000, 0.707107, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.707107, 0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 0.707107, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 1.000000, 0.000000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.707107, 0.707107, 0.000000, -0.000000, 0.707107, 0.707107, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.000000, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.500000, 0.500000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 1.000000, 0.000000, -0.000000 };
static uint smo_uvs_vbo_id = 0; // stencil masking opt uv sphere vertex buffer object id

// init stencil masking optimization UV sphere
static void InitSMOUVS()
{
	glGenBuffers( 1, &smo_uvs_vbo_id );
	glBindBuffer( GL_ARRAY_BUFFER, smo_uvs_vbo_id );
	glBufferData( GL_ARRAY_BUFFER, sizeof(smo_uvs_coords), smo_uvs_coords, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

static void DrawSMOUVS( const point_light_t& light )
{
	const float scale = 1.2;
	r::MultMatrix( mat4_t( light.translation_wspace, mat3_t::GetIdentity(), light.radius*scale ) );

	r::NoShaders();

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
static void CalcViewVector( const camera_t& cam )
{
	int _w = r::w;
	int _h = r::h;
	int pixels[4][2]={ {_w,_h}, {0,_h}, { 0,0 }, {_w,0} }; // from righ up and CC wise to right down, Just like we render the quad
	int viewport[4]={ 0, 0, _w, _h };

	for( int i=0; i<4; i++ )
	{
		/* Original Code:
		r::UnProject( pixels[i][0], pixels[i][1], 10, cam.GetViewMatrix(), cam.GetProjectionMatrix(), viewport,
		              view_vectors[i].x, view_vectors[i].y, view_vectors[i].z );
		view_vectors[i] = cam.GetViewMatrix() * view_vectors[i];
		The original code is the above 3 lines. The optimized follows:*/

		vec3_t vec;
		vec.x = (2.0*(pixels[i][0]-viewport[0]))/viewport[2] - 1.0;
		vec.y = (2.0*(pixels[i][1]-viewport[1]))/viewport[3] - 1.0;
		vec.z = 1.0;

		view_vectors[i] = vec.GetTransformed( cam.GetInvProjectionMatrix() );
		// end of optimized code
	}
}


//=====================================================================================================================================
// CalcPlanes                                                                                                                         =
//=====================================================================================================================================
/// Calc the planes that we will use inside the shader to calculate the frag pos in view space
static void CalcPlanes( const camera_t& cam )
{
	planes.x = -cam.GetZFar() / (cam.GetZFar() - cam.GetZNear());
	planes.y = -cam.GetZFar() * cam.GetZNear() / (cam.GetZFar() - cam.GetZNear());
}


/*
=======================================================================================================================================
InitStageFBO                                                                                                                          =
=======================================================================================================================================
*/
static void InitStageFBO()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// init the stencil render buffer
	glGenRenderbuffers( 1, &stencil_rb );
	glBindRenderbuffer( GL_RENDERBUFFER, stencil_rb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX, r::w, r::h );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_rb );

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the txtrs
	fai.CreateEmpty2D( r::w, r::h, GL_RGB, GL_RGB );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create deferred shading illumination stage FBO" );

	// unbind
	fbo.Unbind();
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	// load the shaders
	shdr_is_ambient = rsrc::shaders.Load( "shaders/is_ap.glsl" );
	shdr_is_lp_point_light = rsrc::shaders.Load( "shaders/is_lp_point.glsl" );
	shdr_is_lp_spot_light_nos = rsrc::shaders.Load( "shaders/is_lp_spot.glsl" );
	shdr_is_lp_spot_light_s = rsrc::shaders.Load( "shaders/is_lp_spot_shad.glsl" );


	// init the rest
	InitStageFBO();
	InitSMOUVS();

	r::is::shadows::Init();
}


/*
=======================================================================================================================================
AmbientPass                                                                                                                           =
=======================================================================================================================================
*/
static void AmbientPass( const camera_t& /*cam*/, const vec3_t& color )
{
	glDisable( GL_BLEND );

	// set the shader
	shdr_is_ambient->Bind();

	// set the uniforms
	glUniform3fv( shdr_is_ambient->GetUniformLocation(0), 1, &((vec3_t)color)[0] );
	shdr_is_ambient->LocTexUnit( shdr_is_ambient->GetUniformLocation(1), r::ms::diffuse_fai, 0 );

	// Draw quad
	r::DrawQuad( shdr_is_ambient->GetAttributeLocation(0) );
}


//=====================================================================================================================================
// SetStencilMask [point light]                                                                                                       =
//=====================================================================================================================================
/// Clears the stencil buffer and draws a shape in the stencil buffer (in this case the shape is a UV shpere)
static void SetStencilMask( const camera_t& cam, const point_light_t& light )
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
	r::SetProjectionViewMatrices( cam );


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


/*
=======================================================================================================================================
SetStencilMask [spot light]                                                                                                           =
see above                                                                                                                             =
=======================================================================================================================================
*/
static void SetStencilMask( const camera_t& cam, const spot_light_t& light )
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
	r::SetProjectionViewMatrices( cam );


	// render camera's shape to stencil buffer
	r::NoShaders();
	const camera_t& lcam = light.camera;
	float x = lcam.GetZFar() / tan( (PI-lcam.GetFovX())/2 );
	float y = tan( lcam.GetFovY()/2 ) * lcam.GetZFar();
	float z = -lcam.GetZFar();

	const int tris_num = 6;

	float verts[tris_num][3][3] = {
		{ { 0.0, 0.0, 0.0 }, { x, -y, z }, { x,  y, z } }, // right triangle
		{ { 0.0, 0.0, 0.0 }, { x,  y, z }, {-x,  y, z } }, // top
		{ { 0.0, 0.0, 0.0 }, {-x,  y, z }, {-x, -y, z } }, // left
		{ { 0.0, 0.0, 0.0 }, {-x, -y, z }, { x, -y, z } }, // bottom
		{ { x, -y, z }, {-x,  y, z }, { x,  y, z } }, // front up right
		{ { x, -y, z }, {-x, -y, z }, {-x,  y, z } }, // front bottom left
	};

	r::MultMatrix( lcam.transformation_wspace );
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
static void PointLightPass( const camera_t& cam, const point_light_t& light )
{
	//** make a check wether the point light passes the frustum test **
	bsphere_t sphere( light.translation_wspace, light.radius );
	if( !cam.InsideFrustum( sphere ) ) return;

	//** set the scissors **
	//int n = SetScissors( cam.GetViewMatrix()*light.translation_wspace, light.radius );
	//if( n < 1 ) return;

	//** stencil optimization **
	SetStencilMask( cam, light );

	//** bind the shader **
	const shader_prog_t& shader = *shdr_is_lp_point_light; // I dont want to type
	shader.Bind();

	// bind the material stage framebuffer attachable images
	shader.LocTexUnit( shader.GetUniformLocation(0), r::ms::normal_fai, 0 );
	shader.LocTexUnit( shader.GetUniformLocation(1), r::ms::diffuse_fai, 1 );
	shader.LocTexUnit( shader.GetUniformLocation(2), r::ms::specular_fai, 2 );
	shader.LocTexUnit( shader.GetUniformLocation(3), r::ms::depth_fai, 3 );
	glUniform2fv( shader.GetUniformLocation(4), 1, &planes[0] );

	vec3_t light_pos_eye_space = light.translation_wspace.GetTransformed( cam.GetViewMatrix() );
	glUniform3fv( shader.GetUniformLocation(5), 1, &light_pos_eye_space[0] );
	glUniform1f( shader.GetUniformLocation(6), 1.0/light.radius );
	glUniform3fv( shader.GetUniformLocation(7), 1, &vec3_t(light.light_props->GetDiffuseColor())[0] );
	glUniform3fv( shader.GetUniformLocation(8), 1, &vec3_t(light.light_props->GetSpecularColor())[0] );

	//** render quad **
	glEnableVertexAttribArray( shader.GetAttributeLocation(0) );
	glEnableVertexAttribArray( shader.GetAttributeLocation(1) );

	glVertexAttribPointer( shader.GetAttributeLocation(0), 2, GL_FLOAT, false, 0, &quad_vert_cords[0] );
	glVertexAttribPointer( shader.GetAttributeLocation(1), 3, GL_FLOAT, false, 0, &view_vectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableVertexAttribArray( shader.GetAttributeLocation(0) );
	glDisableVertexAttribArray( shader.GetAttributeLocation(1) );

	//glDisable( GL_SCISSOR_TEST );
	glDisable( GL_STENCIL_TEST );
}


/*
=======================================================================================================================================
SpotLightPass                                                                                                                         =
=======================================================================================================================================
*/
static void SpotLightPass( const camera_t& cam, const spot_light_t& light )
{
	//** first of all check if the light's camera is inside the frustum **
	if( !cam.InsideFrustum( light.camera ) ) return;

	//** stencil optimization **
	SetStencilMask( cam, light );

	//** generate the shadow map (if needed) **
	if( light.casts_shadow )
	{
		r::is::shadows::RunPass( light.camera );

		// restore the IS FBO
		fbo.Bind();

		// and restore blending and depth test
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
		glDisable( GL_DEPTH_TEST );
	}

	//** set the shader and uniforms **
	const shader_prog_t* shdr; // because of the huge name

	if( light.casts_shadow )  shdr = shdr_is_lp_spot_light_s;
	else                      shdr = shdr_is_lp_spot_light_nos;

	shdr->Bind();

	// bind the framebuffer attachable images
	shdr->LocTexUnit( shdr->GetUniformLocation(0), r::ms::normal_fai, 0 );
	shdr->LocTexUnit( shdr->GetUniformLocation(1), r::ms::diffuse_fai, 1 );
	shdr->LocTexUnit( shdr->GetUniformLocation(2), r::ms::specular_fai, 2 );
	shdr->LocTexUnit( shdr->GetUniformLocation(3), r::ms::depth_fai, 3 );

	if( light.light_props->GetTexture() == NULL )
		ERROR( "No texture is attached to the light. light_props name: " << light.light_props->GetName() );

	// the planes
	//glUniform2fv( shdr->GetUniformLocation("planes"), 1, &planes[0] );
	glUniform2fv( shdr->GetUniformLocation(4), 1, &planes[0] );

	// the light params
	vec3_t light_pos_eye_space = light.translation_wspace.GetTransformed( cam.GetViewMatrix() );
	glUniform3fv( shdr->GetUniformLocation(5), 1, &light_pos_eye_space[0] );
	glUniform1f( shdr->GetUniformLocation(6), 1.0/light.GetDistance() );
	glUniform3fv( shdr->GetUniformLocation(7), 1, &vec3_t(light.light_props->GetDiffuseColor())[0] );
	glUniform3fv( shdr->GetUniformLocation(8), 1, &vec3_t(light.light_props->GetSpecularColor())[0] );

	// set the light texture
	shdr->LocTexUnit( shdr->GetUniformLocation(9), *light.light_props->GetTexture(), 4 );
	// before we render disable anisotropic in the light.texture because it produces artefacts. ToDo: see if this is unececeary in future drivers
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


	//** set texture matrix for shadowmap projection **
	// Bias * P_light * V_light * inv( V_cam )
	//const float mBias[] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
	static mat4_t bias_m4( 0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0 );
	mat4_t tex_projection_mat;
	tex_projection_mat = bias_m4 * light.camera.GetProjectionMatrix() * light.camera.GetViewMatrix() * cam.transformation_wspace;
	glUniformMatrix4fv( shdr->GetUniformLocation(10), 1, true, &tex_projection_mat[0] );

	/*
	const float mBias[] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
	glActiveTexture( GL_TEXTURE0 );
	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( mBias );
	r::MultMatrix( light.camera.GetProjectionMatrix() );
	r::MultMatrix( light.camera.GetViewMatrix() );
	r::MultMatrix( cam.transformation_wspace );
	glMatrixMode(GL_MODELVIEW);*/

	// the shadow stuff
	// render depth to texture and then bind it
	if( light.casts_shadow )
	{
		shdr->LocTexUnit( shdr->GetUniformLocation(11), r::is::shadows::shadow_map, 5 );
	}

	//** render quad **
	glEnableVertexAttribArray( shdr->GetAttributeLocation(0) );
	glEnableVertexAttribArray( shdr->GetAttributeLocation(1) );

	glVertexAttribPointer( shdr->GetAttributeLocation(0), 2, GL_FLOAT, false, 0, &quad_vert_cords[0] );
	glVertexAttribPointer( shdr->GetAttributeLocation(1), 3, GL_FLOAT, false, 0, &view_vectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableVertexAttribArray( shdr->GetAttributeLocation(0) );
	glDisableVertexAttribArray( shdr->GetAttributeLocation(1) );

	// restore texture matrix
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable( GL_STENCIL_TEST );
}


/*
=======================================================================================================================================
RunStage                                                                                                                              =
=======================================================================================================================================
*/
void RunStage( const camera_t& cam )
{
	// FBO
	fbo.Bind();

	// OGL stuff
	r::SetViewport( 0, 0, r::w, r::h );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );

	// ambient pass
	AmbientPass( cam, scene::GetAmbientColor() );

	// light passes
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	CalcViewVector( cam );
	CalcPlanes( cam );

	// for all lights
	for( uint i=0; i<scene::lights.size(); i++ )
	{
		const light_t& light = *scene::lights[i];
		switch( light.type )
		{
			case light_t::LT_POINT:
			{
				const point_light_t& pointl = static_cast<const point_light_t&>(light);
				PointLightPass( cam, pointl );
				break;
			}

			case light_t::LT_SPOT:
			{
				const spot_light_t& projl = static_cast<const spot_light_t&>(light);
				SpotLightPass( cam, projl );
				break;
			}

			default:
				DEBUG_ERR( 1 );
		}
	}

	// FBO
	fbo.Unbind();
}

}} // end namespaces
