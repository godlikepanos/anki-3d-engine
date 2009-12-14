#include "renderer.h"
#include "texture.h"
#include "renderer.h"
#include "lights.h"
#include "scene.h"

extern light_t lights[1];

namespace r {
namespace dfr {

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/

enum mat_pass_fbo_attached_img_e
{
	MPFAI_NORMAL,
	MPFAI_DIFFUSE,
	MPFAI_SPECULAR,
	MPFAI_DEPTH,
	MAT_PASS_FBO_ATTACHED_IMGS_NUM
};

enum illum_pass_fbo_attached_img_e
{
	IPFAI_DIFFUSE,
	ILLUM_PASS_FBO_ATTACHED_IMGS_NUM
};

static GLenum mat_pass_color_attachments[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
const int mat_pass_color_attachments_num = 3;

static GLenum illum_pass_color_attachments[] = { GL_COLOR_ATTACHMENT0_EXT };
const int illum_pass_color_attachments_num = 1;

static uint mat_pass_fbo_id = 0, illum_pass_fbo_id = 0;

// framebuffer attachable images
static texture_t
	mat_pass_fbo_attached_imgs[MAT_PASS_FBO_ATTACHED_IMGS_NUM],
	illum_pass_fbo_attached_imgs[ILLUM_PASS_FBO_ATTACHED_IMGS_NUM];

static shader_prog_t shdr_point_light_pass, shdr_ambient_pass;


// the bellow are used to speedup the calculation of the frag pos (view space) inside the shader. This is done by precompute the
// view vectors each for one corner of the screen and tha planes used to compute the frag_pos_view_space.z from the depth value.
static vec3_t view_vectors[4];
static vec2_t planes;


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

	// create buffers
	const int internal_format = GL_RGBA_FLOAT32_ATI;
	mat_pass_fbo_attached_imgs[MPFAI_NORMAL].CreateEmpty( r::w, r::h, internal_format, GL_RGBA, GL_UNSIGNED_BYTE );
	mat_pass_fbo_attached_imgs[MPFAI_DIFFUSE].CreateEmpty( r::w, r::h, internal_format, GL_RGBA, GL_UNSIGNED_BYTE );
	mat_pass_fbo_attached_imgs[MPFAI_SPECULAR].CreateEmpty( r::w, r::h, internal_format, GL_RGBA, GL_UNSIGNED_BYTE );

	mat_pass_fbo_attached_imgs[MPFAI_DEPTH].CreateEmpty( r::w, r::h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT );

	// attach the buffers to the FBO
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mat_pass_fbo_attached_imgs[MPFAI_NORMAL].gl_id, 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mat_pass_fbo_attached_imgs[MPFAI_DIFFUSE].gl_id, 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mat_pass_fbo_attached_imgs[MPFAI_SPECULAR].gl_id, 0 );

	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_TEXTURE_2D, mat_pass_fbo_attached_imgs[MPFAI_DEPTH].gl_id, 0 );

	// test if success
	if( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT )
		FATAL( "Cannot create deferred shading material pass FBO" );

	// unbind
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
static void InitIllumPassFBO()
{
	// create FBO
	glGenFramebuffersEXT( 1, &illum_pass_fbo_id );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, illum_pass_fbo_id );

	// create the txtrs
	illum_pass_fbo_attached_imgs[IPFAI_DIFFUSE].CreateEmpty( r::w, r::h, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, illum_pass_fbo_attached_imgs[IPFAI_DIFFUSE].gl_id, 0 );

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
	shdr_point_light_pass.Load( "shaders/light_pass_point.shdr" );
	shdr_ambient_pass.Load( "shaders/ambient_pass.shdr" );

	// material pass
	InitMatPassFBO();

	// illumination pass
	InitIllumPassFBO();
}


/*
=======================================================================================================================================
MaterialPass                                                                                                                          =
=======================================================================================================================================
*/
void MaterialPass( const camera_t& cam )
{
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mat_pass_fbo_id );
	glDrawBuffers( mat_pass_color_attachments_num, mat_pass_color_attachments );

	glClear( GL_DEPTH_BUFFER_BIT );
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );
	scene::UpdateAllLights();

	scene::skybox.Render( cam.GetViewMatrix().GetRotationPart() );

	shader_prog_t* shdr = ass::SearchShdr( "no_normmap_no_diffmap.shdr" );
	if( shdr == NULL ) shdr = ass::LoadShdr( "shaders/no_normmap_no_diffmap.shdr" );
	shdr->Bind();

	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );

	scene::RenderAllObjs();

	r::NoShaders();

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
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
AmbientPass                                                                                                                           =
=======================================================================================================================================
*/
void AmbientPass( const camera_t& cam, const vec3_t& color )
{
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glPolygonMode( GL_FRONT, GL_FILL );

	// set the shader
	shdr_ambient_pass.Bind();

	glUniform3fv( shdr_ambient_pass.GetUniLocation("ambient_color"), 1, &((vec3_t)color)[0] );

	mat_pass_fbo_attached_imgs[MPFAI_DIFFUSE].BindToTxtrUnit(0);  glUniform1i( shdr_ambient_pass.GetUniLocation("ambient_map"), 0 );
	mat_pass_fbo_attached_imgs[MPFAI_DEPTH].BindToTxtrUnit(1);  glUniform1i( shdr_ambient_pass.GetUniLocation("depth_map"), 1 );

	glUniform2fv( shdr_ambient_pass.GetUniLocation("camerarange"), 1, &(vec2_t(cam.GetZNear(), cam.GetZFar()))[0] );

	glUniform2fv( shdr_ambient_pass.GetUniLocation("screensize"), 1, &(vec2_t(r::w, r::w))[0] );


	float points [][2] = { {r::w,r::h}, {0,r::h}, {0,0}, {r::w,0} };
	float uvs [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };

	mat_pass_fbo_attached_imgs[MPFAI_DIFFUSE].BindToTxtrUnit(0);

	glBegin( GL_QUADS );
		glTexCoord2fv( uvs[0] );
		glVertex2fv( points[0] );
		glTexCoord2fv( uvs[1] );
		glVertex2fv( points[1] );
		glTexCoord2fv( uvs[2] );
		glVertex2fv( points[2] );
		glTexCoord2fv( uvs[3] );
		glVertex2fv( points[3] );
	glEnd();
}


/*
=======================================================================================================================================
LightingPass                                                                                                                          =
=======================================================================================================================================
*/
void LightingPass( const camera_t& cam )
{
	CalcViewVector( cam );
	CalcPlanes( cam );

	shdr_point_light_pass.Bind();

	// bind textures
	for( int i=0; i<MAT_PASS_FBO_ATTACHED_IMGS_NUM; i++ )
	{
		mat_pass_fbo_attached_imgs[i].BindToTxtrUnit(i);
		glUniform1i( shdr_point_light_pass.uniform_locations[i], i );
	}

	// other uniforms
	glUniform2fv( shdr_point_light_pass.GetUniLocation("planes"), 1, &planes[0] );

	vec3_t light_pos_eye_space = cam.GetViewMatrix() * lights[0].world_translation;
	glUniform3fv( shdr_point_light_pass.GetUniLocation("light_pos_eyespace"), 1, &light_pos_eye_space[0] );

	glUniform1f( shdr_point_light_pass.GetUniLocation("inv_light_radius"), 1/4.0 );

	// render
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );

	int loc = shdr_point_light_pass.GetAttrLocation( "view_vector" );

	float points [][2] = { {r::w,r::h}, {0,r::h}, {0,0}, {r::w,0} };
	float uvs [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };

	glBegin( GL_QUADS );
		glVertexAttrib3fv( loc, &(view_vectors[0])[0] );
		glTexCoord2fv( uvs[0] );
		glVertex2fv( points[0] );
		glVertexAttrib3fv( loc, &(view_vectors[1])[0] );
		glTexCoord2fv( uvs[1] );
		glVertex2fv( points[1] );
		glVertexAttrib3fv( loc, &(view_vectors[2])[0] );
		glTexCoord2fv( uvs[2] );
		glVertex2fv( points[2] );
		glVertexAttrib3fv( loc, &(view_vectors[3])[0] );
		glTexCoord2fv( uvs[3] );
		glVertex2fv( points[3] );
	glEnd();

}


/*
=======================================================================================================================================
IlluminationPass                                                                                                                      =
=======================================================================================================================================
*/
void IlluminationPass( const camera_t& cam, const vec3_t& color )
{

}



}} // end namespaces
