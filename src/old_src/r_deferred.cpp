#include "renderer.h"

#include "texture.h"
#include "lights.h"
#include "scene.h"
#include "assets.h"
#include "collision.h"
#include "r_private.h"

namespace r {
namespace dfr {

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/

static GLenum mat_pass_color_attachments[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
const int mat_pass_color_attachments_num = 3;

static GLenum illum_pass_color_attachments[] = { GL_COLOR_ATTACHMENT0_EXT };
const int illum_pass_color_attachments_num = 1;

// FBOs
static uint mat_pass_fbo_id = 0, illum_pass_fbo_id = 0;

// framebuffer attachable images
const int MAT_PASS_FBO_ATTACHED_IMGS_NUM = 4;

texture_t ipfai_illum_scene;
texture_t mpfai_normal, mpfai_diffuse, mpfai_specular, mpfai_depth;

// stencil render target for illum stage
static uint ip_stencil_rb;

// shaders
static shader_prog_t* shdr_ambient_pass;
static shader_prog_t* shdr_point_light_pass;
static shader_prog_t* shdr_proj_light_nos_pass;
static shader_prog_t* shdr_proj_light_s_pass;
static shader_prog_t* shdr_post_proc_stage;

// default material
static material_t* dflt_material;


// the bellow are used to speedup the calculation of the frag pos (view space) inside the shader. This is done by precompute the
// view vectors each for one corner of the screen and tha planes used to compute the frag_pos_view_space.z from the depth value.
static vec3_t view_vectors[4];
static vec2_t planes;


// to draw the quad in the screen
static float quad_points [][2] = { {r::w,r::h}, {0,r::h}, {0,0}, {r::w,0} };
static float quad_uvs [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };


/*
=======================================================================================================================================
stencil masking opt uv sphere                                                                                                         =
=======================================================================================================================================
*/
static float smo_uvs_coords [] = { -0.000000, 0.000000, -1.000000, 0.500000, 0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.707107, 0.000000, 0.707107, -0.000000, 0.707107, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.000000, 0.707107, -0.707107, 0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.000000, 0.707107, 0.707107, -0.707107, -0.000000, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.707107, -0.000000, -0.707107, -0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, -0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.707107, -0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, -0.500000, -0.500000, -0.707107, -0.000000, 0.000000, -1.000000, 0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.707107, 0.000000, -0.707107, 0.500000, -0.500000, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 1.000000, 0.000000, -0.000000, 0.500000, -0.500000, -0.707107, 1.000000, 0.000000, -0.000000, 0.707107, -0.707107, 0.000000, 0.707107, -0.707107, 0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.000000, 0.707107, 0.707107, -0.707107, 0.000000, 0.707107, 0.000000, 0.707107, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.707107, -0.707107, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, -0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, -0.707107, 0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, 0.707107, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.500000, -0.500000, 0.707107, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, -0.707107, -0.707107, -0.707107, 0.000000, -0.707107, -0.000000, -0.707107, -0.707107, -0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -1.000000, -0.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.707107, -0.000000, 0.707107, -0.707107, 0.707107, 0.000000, -0.707107, -0.000000, 0.707107, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.707107, 0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 0.707107, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 1.000000, 0.000000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.707107, 0.707107, 0.000000, -0.000000, 0.707107, 0.707107, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.000000, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.500000, 0.500000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 1.000000, 0.000000, -0.000000 };
static uint smo_uvs_vbo_id = NULL; // stencil masking opt uv sphere vertex buffer object id

static void InitSMOUVS()
{
	glGenBuffers( 1, &smo_uvs_vbo_id );
	glBindBuffer( GL_ARRAY_BUFFER, smo_uvs_vbo_id );
	glBufferData( GL_ARRAY_BUFFER, sizeof(smo_uvs_coords), smo_uvs_coords, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

static void DrawSMOUVS( const point_light_t& light )
{
	r::MultMatrix( mat4_t( light.translation_wspace, mat3_t::ident, light.radius*1.1 ) );

	r::NoShaders();

	glBindBuffer( GL_ARRAY_BUFFER, smo_uvs_vbo_id );
	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDrawArrays( GL_TRIANGLES, 0, sizeof(smo_uvs_coords)/sizeof(float)/3 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

/*
=======================================================================================================================================
DrawScreenQuad                                                                                                                        =
=======================================================================================================================================
*/
inline void DrawScreenQuad()
{
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 2, GL_FLOAT, 0, quad_points );
	glTexCoordPointer( 2, GL_FLOAT, 0, quad_uvs );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}

/*
=======================================================================================================================================
DrawScreenQuadAttr                                                                                                                    =
=======================================================================================================================================
*/
inline void DrawScreenQuadAttr( uint loc )
{
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableVertexAttribArray( loc );

	glVertexPointer( 2, GL_FLOAT, 0, quad_points );
	glTexCoordPointer( 2, GL_FLOAT, 0, quad_uvs );
	glVertexAttribPointer( loc, 3, GL_FLOAT, 0, 0, &view_vectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableVertexAttribArray( loc );
}


/*
=======================================================================================================================================
InitMatPassFBO                                                                                                                        =
=======================================================================================================================================
*/
static void InitMatPassFBO()
{
	// create FBO
	glGenFramebuffersEXT( 1, &mat_pass_fbo_id );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mat_pass_fbo_id );

	// inform in what buffers we draw
	glDrawBuffers( mat_pass_color_attachments_num, mat_pass_color_attachments );

	// create buffers
	const int internal_format = GL_RGBA_FLOAT16_ATI;
	mpfai_normal.CreateEmpty( r::w, r::h, internal_format, GL_RGBA, GL_UNSIGNED_BYTE );
	mpfai_diffuse.CreateEmpty( r::w, r::h, internal_format, GL_RGBA, GL_UNSIGNED_BYTE );
	mpfai_specular.CreateEmpty( r::w, r::h, internal_format, GL_RGBA, GL_UNSIGNED_BYTE );

	mpfai_depth.CreateEmpty( r::w, r::h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT );

	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mpfai_normal.gl_id, 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mpfai_diffuse.gl_id, 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mpfai_specular.gl_id, 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, mpfai_depth.gl_id, 0 );

	// test if success
	if( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
		FATAL( "Cannot create deferred shading material pass FBO" );

	// unbind
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}


/*
=======================================================================================================================================
InitIllumPassFBO                                                                                                                      =
=======================================================================================================================================
*/
static void InitIllumPassFBO()
{
	// create FBO
	glGenFramebuffersEXT( 1, &illum_pass_fbo_id );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, illum_pass_fbo_id );

	// init the stencil render buffer
	glGenRenderbuffersEXT( 1, &ip_stencil_rb );
	glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, ip_stencil_rb );
	glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX, r::w, r::h );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, ip_stencil_rb );

	// inform in what buffers we draw
	glDrawBuffers( illum_pass_color_attachments_num, illum_pass_color_attachments );

	// create the txtrs
	ipfai_illum_scene.CreateEmpty( r::w, r::h, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ipfai_illum_scene.gl_id, 0 );

	// test if success
	if( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
		FATAL( "Cannot create deferred shading illumination pass FBO" );

	// unbind
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	// the shaders
	shdr_ambient_pass = ass::LoadShdr( "shaders/ambient_pass.shdr" );
	shdr_point_light_pass = ass::LoadShdr( "shaders/lp_point.shdr" );
	shdr_proj_light_nos_pass = ass::LoadShdr( "shaders/lp_proj.shdr" );
	shdr_proj_light_s_pass = ass::LoadShdr( "shaders/lp_proj_shad.shdr" );
	shdr_post_proc_stage = ass::LoadShdr( "shaders/pp_stage.shdr" );

	// the default material
	dflt_material = ass::LoadMat( "materials/default.mat" );

	// init FBOs
	InitMatPassFBO();
	InitIllumPassFBO();

	InitSMOUVS();
}


/*
=======================================================================================================================================
CalcViewVector                                                                                                                        =
=======================================================================================================================================
*/
static void CalcViewVector( const camera_t& cam )
{
	int pixels[4][2]={ {r::w,r::h}, {0,r::h}, { 0,0 }, {r::w,0} }; // from righ up and CC wise to right down, Just like we render the quad
	int viewport[4]={ 0, 0, r::w, r::h };

	mat3_t view_rotation = cam.GetViewMatrix().GetRotationPart();

	for( int i=0; i<4; i++ )
	{
		/* Original Code:
		r::UnProject( pixels[i][0], pixels[i][1], 10, cam.GetViewMatrix(), cam.GetProjectionMatrix(), viewport,
		              view_vectors[i].x, view_vectors[i].y, view_vectors[i].z );
		view_vectors[i] = cam.GetViewMatrix() * view_vectors[i];
		The original code is the above 3 lines. The optimized follows:*/

		mat4_t inv_pm = cam.GetProjectionMatrix().Inverted();

		vec4_t vec;
		vec.x = (2.0*(pixels[i][0]-viewport[0]))/viewport[2] - 1.0;
		vec.y = (2.0*(pixels[i][1]-viewport[1]))/viewport[3] - 1.0;
		vec.z = 2.0*10.0 - 1.0;
		vec.w = 1.0;

		view_vectors[i] = vec3_t(inv_pm * vec);
		// end of optimized code
	}
}


/*
=======================================================================================================================================
CalcPlanes                                                                                                                            =
=======================================================================================================================================
*/
static void CalcPlanes( const camera_t& cam )
{
	planes.x = -cam.GetZFar() / (cam.GetZFar() - cam.GetZNear());
	planes.y = -cam.GetZFar() * cam.GetZNear() / (cam.GetZFar() - cam.GetZNear());
}


/*
=======================================================================================================================================
MaterialStage                                                                                                                         =
=======================================================================================================================================
*/
void MaterialStage( const camera_t& cam )
{
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mat_pass_fbo_id );

	glClear( GL_DEPTH_BUFFER_BIT );
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );

	scene::skybox.Render( cam.GetViewMatrix().GetRotationPart() );

	for( uint i=0; i<scene::meshes.size(); i++ )
	{
		mesh_t* mesh = scene::meshes[i];

		DEBUG_ERR( !mesh->material );

		mesh->material->UseMaterialPass();

		mesh->Render();
	}

	r::NoShaders();

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}


/*
=======================================================================================================================================
AmbientPass                                                                                                                           =
=======================================================================================================================================
*/
static void AmbientPass( const camera_t& cam, const vec3_t& color )
{
	glDisable( GL_BLEND );

	// set the shader
	shdr_ambient_pass->Bind();

	//glUniform3fv( shdr_ambient_pass->GetUniLocation("ambient_color"), 1, &((vec3_t)color)[0] );
	glUniform3fv( shdr_ambient_pass->uniform_locations[0], 1, &((vec3_t)color)[0] );

	/*mpfai_diffuse.Bind(0);  glUniform1i( shdr_ambient_pass->GetUniLocation("ambient_map"), 0 );
	mpfai_depth.Bind(1);  glUniform1i( shdr_ambient_pass->GetUniLocation("depth_map"), 1 );*/

	mpfai_diffuse.Bind(0);  glUniform1i( shdr_ambient_pass->uniform_locations[1], 0 );
	mpfai_depth.Bind(1);  glUniform1i( shdr_ambient_pass->uniform_locations[2], 1 );

	DrawScreenQuad();
}


/*
=======================================================================================================================================
SetScissors                                                                                                                           =
the function sets the scissors and returns the pixels that are in rect region                                                         =
=======================================================================================================================================
*/
static int SetScissors( const vec3_t& light_pos_vspace, float radius )
{
	int rect[4]={ 0,0,r::w,r::w };
	float d;

	const float& r = radius;
	float r2 = r*r;

	const vec3_t& l = light_pos_vspace;
	vec3_t l2 = light_pos_vspace * light_pos_vspace;

	float aspect = r::aspect_ratio;

	float e1 = 1.2f;
	float e2 = 1.2f * aspect;

	d = r2*l2.x - (l2.x+l2.z)*(r2-l2.z);

	if (d>=0)
	{
		d=Sqrt(d);

		float nx1=(r*l.x + d)/(l2.x+l2.z);
		float nx2=(r*l.x - d)/(l2.x+l2.z);

		float nz1=(r-nx1*l.x)/l.z;
		float nz2=(r-nx2*l.x)/l.z;

		//float e=1.25f;
		//float a=aspect;

		float pz1=(l2.x+l2.z-r2)/(l.z-(nz1/nx1)*l.x);
		float pz2=(l2.x+l2.z-r2)/(l.z-(nz2/nx2)*l.x);

		if (pz1<0)
		{
			float fx=nz1*e1/nx1;
			int ix=(int)((fx+1.0f)*r::w*0.5f);

			float px=-pz1*nz1/nx1;
			if (px<l.x)
				rect[0]=max(rect[0],ix);
			else
				rect[2]=min(rect[2],ix);
		}

		if (pz2<0)
		{
			float fx=nz2*e1/nx2;
			int ix=(int)((fx+1.0f)*r::w*0.5f);

			float px=-pz2*nz2/nx2;
			if (px<l.x)
				rect[0]=max(rect[0],ix);
			else
				rect[2]=min(rect[2],ix);
		}
	}

	d=r2*l2.y - (l2.y+l2.z)*(r2-l2.z);
	if (d>=0)
	{
		d=Sqrt(d);

		float ny1=(r*l.y + d)/(l2.y+l2.z);
		float ny2=(r*l.y - d)/(l2.y+l2.z);

		float nz1=(r-ny1*l.y)/l.z;
		float nz2=(r-ny2*l.y)/l.z;

		float pz1=(l2.y+l2.z-r2)/(l.z-(nz1/ny1)*l.y);
		float pz2=(l2.y+l2.z-r2)/(l.z-(nz2/ny2)*l.y);

		if (pz1<0)
		{
			float fy=nz1*e2/ny1;
			int iy=(int)((fy+1.0f)*r::h*0.5f);

			float py=-pz1*nz1/ny1;
			if (py<l.y)
				rect[1]=max(rect[1],iy);
			else
				rect[3]=min(rect[3],iy);
		}

		if (pz2<0)
		{
			float fy=nz2*e2/ny2;
			int iy=(int)((fy+1.0f)*r::h*0.5f);

			float py=-pz2*nz2/ny2;
			if (py<l.y)
				rect[1]=max(rect[1],iy);
			else
				rect[3]=min(rect[3],iy);
		}
	}

	int n = ( rect[2]-rect[0])*(rect[3]-rect[1] );
	if( n<=0 )
		return 0;
	if( (uint)n == r::w*r::h )
	{
		glDisable(GL_SCISSOR_TEST);
		return r::w*r::h;
	}

	glScissor(rect[0],rect[1],rect[2]-rect[0],rect[3]-rect[1]);
	glEnable(GL_SCISSOR_TEST);

	//INFO( rect[0] << ' ' << rect[1] << ' ' << rect[2]-rect[0] << ' ' << rect[3]-rect[1] );
	return n;
}


/*
=======================================================================================================================================
SetStencilMask [point light]                                                                                                          =
=======================================================================================================================================
*/
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
SetStencilMask [projected light]                                                                                                      =
=======================================================================================================================================
*/
static void SetStencilMask( const camera_t& cam, const proj_light_t& light )
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
	///** make a check wether the point light passes the frustum test **
	bsphere_t sphere( light.translation_wspace, light.radius );
	if( !cam.InsideFrustum( sphere ) ) return;

	///** set the scissors **
	//int n = SetScissors( cam.GetViewMatrix()*light.translation_wspace, light.radius );
	//if( n < 1 ) return;

	///** stencil optimization **
	SetStencilMask( cam, light );

	///** bind the shader **
	shdr_point_light_pass->Bind();

	// bind the framebuffer attachable images
	mpfai_normal.Bind(0);
	mpfai_diffuse.Bind(1);
	mpfai_specular.Bind(2);
	mpfai_depth.Bind(3);
	glUniform1i( shdr_point_light_pass->uniform_locations[0], 0 );
	glUniform1i( shdr_point_light_pass->uniform_locations[1], 1 );
	glUniform1i( shdr_point_light_pass->uniform_locations[2], 2 );
	glUniform1i( shdr_point_light_pass->uniform_locations[3], 3 );

	// set other shader params
	glUniform2fv( shdr_point_light_pass->GetUniLocation("planes"), 1, &planes[0] );

	vec3_t light_pos_eye_space = cam.GetViewMatrix() * light.translation_wspace;

	glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1/light.radius)[0] );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( light.GetDiffuseColor(), 1.0 ))[0] );
	glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( light.GetSpecularColor(), 1.0 ))[0] );

	///** render quad **
	int loc = shdr_point_light_pass->attribute_locations[0]; // view_vector
	DrawScreenQuadAttr( loc );

	//glDisable( GL_SCISSOR_TEST );
	glDisable( GL_STENCIL_TEST );
}


/*
=======================================================================================================================================
ProjLightPass                                                                                                                         =
=======================================================================================================================================
*/
static void ProjLightPass( const camera_t& cam, const proj_light_t& light )
{
	///** first of all check if the light's camera is inside the frustum **
	if( !cam.InsideFrustum( light.camera ) ) return;

	///** stencil optimization **
	SetStencilMask( cam, light );

	///** generate the shadow map (if needed) **
	if( light.casts_shadow )
	{
		shadows::RenderSceneOnlyDepth( light.camera );

		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, illum_pass_fbo_id );
		glDrawBuffers( illum_pass_color_attachments_num, illum_pass_color_attachments );

		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );

		glDisable( GL_DEPTH_TEST );
	}

	///** set texture matrix for shadowmap projection **
	const float mBias[] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
	glActiveTexture( GL_TEXTURE0 );
	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( mBias );
	r::MultMatrix( light.camera.GetProjectionMatrix() );
	r::MultMatrix( light.camera.GetViewMatrix() );
	r::MultMatrix( cam.transformation_wspace );
	glMatrixMode(GL_MODELVIEW);

	///** set the shader and uniforms **
	{
		const shader_prog_t* shdr; // because of the huge name

		if( light.casts_shadow )  shdr = shdr_proj_light_s_pass;
		else                      shdr = shdr_proj_light_nos_pass;

		shdr->Bind();

		// bind the framebuffer attachable images
		mpfai_normal.Bind(0);
		mpfai_diffuse.Bind(1);
		mpfai_specular.Bind(2);
		mpfai_depth.Bind(3);
		glUniform1i( shdr->uniform_locations[0], 0 );
		glUniform1i( shdr->uniform_locations[1], 1 );
		glUniform1i( shdr->uniform_locations[2], 2 );
		glUniform1i( shdr->uniform_locations[3], 3 );

		DEBUG_ERR( light.texture == NULL ); // No texture attached to the light

		light.texture->Bind( MAT_PASS_FBO_ATTACHED_IMGS_NUM );
		//glUniform1i( shdr->GetUniLocation("light_txtr"), MAT_PASS_FBO_ATTACHED_IMGS_NUM );
		glUniform1i( shdr->uniform_locations[4], MAT_PASS_FBO_ATTACHED_IMGS_NUM );

		// the planes
		//glUniform2fv( shdr->GetUniLocation("planes"), 1, &planes[0] );
		glUniform2fv( shdr->uniform_locations[5], 1, &planes[0] );

		// the pos and max influence distance
		vec3_t light_pos_eye_space = cam.GetViewMatrix() * light.translation_wspace;
		glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1.0/light.GetDistance())[0] );

		// the colors
		glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( light.GetDiffuseColor(), 1.0 ))[0] );
		glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( light.GetSpecularColor(), 1.0 ))[0] );

		// the shadow stuff
		// render depth to texture and then bind it
		if( light.casts_shadow )
		{
			r::shadows::shadow_map.Bind( MAT_PASS_FBO_ATTACHED_IMGS_NUM+1 );
			//glUniform1i( shdr->GetUniLocation("shadow_depth_map"), MAT_PASS_FBO_ATTACHED_IMGS_NUM+1 );
			glUniform1i( shdr->uniform_locations[6], MAT_PASS_FBO_ATTACHED_IMGS_NUM+1 );
		}
	}

	///** render quad **
	int loc = shdr_proj_light_nos_pass->attribute_locations[0]; // view_vector
	DrawScreenQuadAttr( loc );

	// restore texture matrix
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable( GL_STENCIL_TEST );
}


/*
=======================================================================================================================================
IlluminationStage                                                                                                                     =
=======================================================================================================================================
*/
void IlluminationStage( const camera_t& cam )
{
	// FBO
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, illum_pass_fbo_id );

	// OGL stuff
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, r::w, 0.0, r::h, -1.0, 1.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_DEPTH_TEST );
	glPolygonMode( GL_FRONT, GL_FILL );

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
		switch( light.GetType() )
		{
			case light_t::POINT:
			{
				const point_light_t& pointl = static_cast<const point_light_t&>(light);
				PointLightPass( cam, pointl );
				break;
			}

			case light_t::PROJ_TXTR:
			{
				const proj_light_t& projl = static_cast<const proj_light_t&>(light);
				ProjLightPass( cam, projl );
				break;
			}

			default:
				FATAL( "Check code" );
		}
	}

	// FBO
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}


/*
=======================================================================================================================================
PostProcStage                                                                                                                         =
=======================================================================================================================================
*/
void PostProcStage( const camera_t& cam )
{
	// set GL
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glPolygonMode( GL_FRONT, GL_FILL );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, r::w, 0.0, r::h, -1.0, 1.0 );

	// set shader
	shdr_post_proc_stage->Bind();

	static texture_t* noise_map = ass::LoadTxtr( "gfx/noise.tga" );
	noise_map->TexParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noise_map->TexParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );


	ipfai_illum_scene.Bind(0);
	glUniform1i( shdr_post_proc_stage->GetUniLocation("illum_scene_map"), 0 );
	mpfai_depth.Bind(1);
	glUniform1i( shdr_post_proc_stage->GetUniLocation("depth_map"), 1 );


	noise_map->Bind(2);
	glUniform1i( shdr_post_proc_stage->GetUniLocation("noise_map"), 2 );

	glUniform2fv( shdr_post_proc_stage->GetUniLocation("camerarange"), 1, &(vec2_t(cam.GetZNear(), cam.GetZFar()))[0] );
	glUniform2fv( shdr_post_proc_stage->GetUniLocation("screensize"), 1, &(vec2_t(r::w, r::w))[0] );

	glBegin( GL_QUADS );
		glTexCoord2fv( quad_uvs[0] );
		glVertex2fv( quad_points[0] );
		glTexCoord2fv( quad_uvs[1] );
		glVertex2fv( quad_points[1] );
		glTexCoord2fv( quad_uvs[2] );
		glVertex2fv( quad_points[2] );
		glTexCoord2fv( quad_uvs[3] );
		glVertex2fv( quad_points[3] );
	glEnd();

	shdr_post_proc_stage->UnBind();
}



}} // end namespaces
