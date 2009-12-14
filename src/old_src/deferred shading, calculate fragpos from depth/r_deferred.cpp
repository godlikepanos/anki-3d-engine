#include "renderer.h"

#include "texture.h"
#include "lights.h"
#include "scene.h"
#include "assets.h"
#include "collision.h"

namespace r {

namespace shadows
{
	extern texture_t depth_map;
	extern void RenderSceneOnlyDepth( const camera_t& cam );
}


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

static texture_t ipfai_illum_scene;

texture_t mpfai_normal, mpfai_diffuse, mpfai_specular, mpfai_depth;

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
	shdr_point_light_pass = ass::LoadShdr( "shaders/light_pass_point.shdr" );
	shdr_proj_light_nos_pass = ass::LoadShdr( "shaders/light_pass_proj_nos.shdr" );
	shdr_proj_light_s_pass = ass::LoadShdr( "shaders/light_pass_proj_s.shdr" );
	shdr_post_proc_stage = ass::LoadShdr( "shaders/post_proc_stage.shdr" );

	// the default material
	dflt_material = ass::LoadMat( "materials/dflt.mat" );

	// init FBOs
	InitMatPassFBO();
	InitIllumPassFBO();
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
	glDrawBuffers( mat_pass_color_attachments_num, mat_pass_color_attachments );

	glClear( GL_DEPTH_BUFFER_BIT );
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, r::w, r::h );

	scene::skybox.Render( cam.GetViewMatrix().GetRotationPart() );

	for( uint i=0; i<scene::objects.size(); i++ )
	{
		object_t* obj = scene::objects[i];

		if( !obj->material ) continue;


		obj->material->UseMaterialPass();


		scene::objects[i]->Render();
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

	glUniform3fv( shdr_ambient_pass->GetUniLocation("ambient_color"), 1, &((vec3_t)color)[0] );

	mpfai_diffuse.BindToTxtrUnit(0);  glUniform1i( shdr_ambient_pass->GetUniLocation("ambient_map"), 0 );
	mpfai_depth.BindToTxtrUnit(1);  glUniform1i( shdr_ambient_pass->GetUniLocation("depth_map"), 1 );

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
}


/*
=======================================================================================================================================
PointLightPass                                                                                                                        =
=======================================================================================================================================
*/
static void PointLightPass( const camera_t& cam, const point_light_t& pointl )
{
	//** make a check wether the point light passes the frustum test **
	bsphere_t sphere( pointl.world_translation, pointl.radius );
	if( !cam.InsideFrustum( sphere ) ) return;
//	proj_light_t* projl = (proj_light_t*)scene::objects[7];
//	if( !projl->camera.InsideFrustum( sphere ) ) return;


	//** bind the shader **
	shdr_point_light_pass->Bind();

	// bind the framebuffer attachable images
	mpfai_normal.BindToTxtrUnit(0);
	mpfai_diffuse.BindToTxtrUnit(1);
	mpfai_specular.BindToTxtrUnit(2);
	mpfai_depth.BindToTxtrUnit(3);
	glUniform1i( shdr_point_light_pass->uniform_locations[0], 0 );
	glUniform1i( shdr_point_light_pass->uniform_locations[1], 1 );
	glUniform1i( shdr_point_light_pass->uniform_locations[2], 2 );
	glUniform1i( shdr_point_light_pass->uniform_locations[3], 3 );

	// set other shader params
	glUniform2fv( shdr_point_light_pass->GetUniLocation("planes"), 1, &planes[0] );

	vec3_t light_pos_eye_space = cam.GetViewMatrix() * pointl.world_translation;

	glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1/pointl.radius)[0] );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( pointl.GetDiffuseColor(), 1.0 ))[0] );
	glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( pointl.GetSpecularColor(), 1.0 ))[0] );

	//** render quad **
	int loc = shdr_point_light_pass->GetAttrLocation( "view_vector" );

	glBegin( GL_QUADS );
		glVertexAttrib3fv( loc, &(view_vectors[0])[0] );
		glTexCoord2fv( quad_uvs[0] );
		glVertex2fv( quad_points[0] );
		glVertexAttrib3fv( loc, &(view_vectors[1])[0] );
		glTexCoord2fv( quad_uvs[1] );
		glVertex2fv( quad_points[1] );
		glVertexAttrib3fv( loc, &(view_vectors[2])[0] );
		glTexCoord2fv( quad_uvs[2] );
		glVertex2fv( quad_points[2] );
		glVertexAttrib3fv( loc, &(view_vectors[3])[0] );
		glTexCoord2fv( quad_uvs[3] );
		glVertex2fv( quad_points[3] );
	glEnd();

}


/*
=======================================================================================================================================
ProjLightPass                                                                                                                         =
=======================================================================================================================================
*/
static void ProjLightPass( const camera_t& cam, const proj_light_t& light )
{
	//** first of all check if the light's camera is inside the frustum **
	if( !cam.InsideFrustum( light.camera ) ) return;

	//** secondly generate the shadow map (if needed) **
	if( light.casts_shadow )
	{
		shadows::RenderSceneOnlyDepth( light.camera );

		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, illum_pass_fbo_id );
		glDrawBuffers( illum_pass_color_attachments_num, illum_pass_color_attachments );

		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );

		glDisable( GL_DEPTH_TEST );
	}

	//** set texture matrix for shadowmap projection **
	const float mBias[] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
	glActiveTexture( GL_TEXTURE0 );
	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( mBias );
	r::MultMatrix( light.camera.GetProjectionMatrix() );
	r::MultMatrix( light.camera.GetViewMatrix() );
	r::MultMatrix( cam.world_transformation );
	glMatrixMode(GL_MODELVIEW);

	//** set the shader and uniforms **
	{
		const shader_prog_t* shdr; // because of the huge name

		if( light.casts_shadow )  shdr = shdr_proj_light_s_pass;
		else                      shdr = shdr_proj_light_nos_pass;

		shdr->Bind();

		// bind the framebuffer attachable images
		mpfai_normal.BindToTxtrUnit(0);
		mpfai_diffuse.BindToTxtrUnit(1);
		mpfai_specular.BindToTxtrUnit(2);
		mpfai_depth.BindToTxtrUnit(3);
		glUniform1i( shdr->uniform_locations[0], 0 );
		glUniform1i( shdr->uniform_locations[1], 1 );
		glUniform1i( shdr->uniform_locations[2], 2 );
		glUniform1i( shdr->uniform_locations[3], 3 );

		DEBUG_ERR( light.texture == NULL ); // No texture attached to the light

		light.texture->BindToTxtrUnit( MAT_PASS_FBO_ATTACHED_IMGS_NUM );
		glUniform1i( shdr->GetUniLocation("light_txtr"), MAT_PASS_FBO_ATTACHED_IMGS_NUM );

		// the planes
		glUniform2fv( shdr->GetUniLocation("planes"), 1, &planes[0] );

		// the pos and max influence distance
		vec3_t light_pos_eye_space = cam.GetViewMatrix() * light.world_translation;
		glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1.0/light.GetDistance())[0] );

		// the colors
		glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( light.GetDiffuseColor(), 1.0 ))[0] );
		glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( light.GetSpecularColor(), 1.0 ))[0] );

		// the shadow stuff
		// render depth to texture and then bind it
		if( light.casts_shadow )
		{
			shadows::depth_map.BindToTxtrUnit( MAT_PASS_FBO_ATTACHED_IMGS_NUM+1 );
			glUniform1i( shdr->GetUniLocation("shadow_depth_map"), MAT_PASS_FBO_ATTACHED_IMGS_NUM+1 );

			glUniform1f( shdr->GetUniLocation("shadow_resolution"), shadows::shadow_resolution );
		}
	}

	// render quad
	int loc = shdr_proj_light_nos_pass->GetAttrLocation( "view_vector" );

	glBegin( GL_QUADS );
		glVertexAttrib3fv( loc, &(view_vectors[0])[0] );
		glTexCoord2fv( quad_uvs[0] );
		glVertex2fv( quad_points[0] );
		glVertexAttrib3fv( loc, &(view_vectors[1])[0] );
		glTexCoord2fv( quad_uvs[1] );
		glVertex2fv( quad_points[1] );
		glVertexAttrib3fv( loc, &(view_vectors[2])[0] );
		glTexCoord2fv( quad_uvs[2] );
		glVertex2fv( quad_points[2] );
		glVertexAttrib3fv( loc, &(view_vectors[3])[0] );
		glTexCoord2fv( quad_uvs[3] );
		glVertex2fv( quad_points[3] );
	glEnd();

	// restore texture matrix
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
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
	glDrawBuffers( illum_pass_color_attachments_num, illum_pass_color_attachments );

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

	ipfai_illum_scene.BindToTxtrUnit(0);
	glUniform1i( shdr_post_proc_stage->GetUniLocation("illum_scene_map"), 0 );
	mpfai_depth.BindToTxtrUnit(1);
	glUniform1i( shdr_post_proc_stage->GetUniLocation("depth_map"), 1 );

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
