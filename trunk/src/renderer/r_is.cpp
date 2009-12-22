/*
The file contains functions and vars used for the deferred shading illumination stage.
*/

#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "lights.h"
#include "resource.h"
#include "scene.h"
#include "r_private.h"
#include "fbo.h"

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
static uint smo_uvs_vbo_id = NULL; // stencil masking opt uv sphere vertex buffer object id

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


/*
=======================================================================================================================================
CalcViewVector                                                                                                                        =
calc the view vector that we will use inside the shader to calculate the frag pos in view space                                       =
=======================================================================================================================================
*/
static void CalcViewVector( const camera_t& cam )
{
	float _w = r::w * r::rendering_quality;
	float _h = r::h * r::rendering_quality;
	int pixels[4][2]={ {_w,_h}, {0.0,_h}, { 0.0,0.0 }, {_w,0.0} }; // from righ up and CC wise to right down, Just like we render the quad
	int viewport[4]={ 0.0, 0.0, _w, _h };

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

		view_vectors[i] = vec.Transformed( cam.GetInvProjectionMatrix() );
		// end of optimized code
	}
}


/*
=======================================================================================================================================
CalcPlanes                                                                                                                            =
calc the planes that we will use inside the shader to calculate the frag pos in view space                                            =
=======================================================================================================================================
*/
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
	glRenderbufferStorage( GL_RENDERBUFFER, GL_STENCIL_INDEX, r::w * r::rendering_quality, r::h * r::rendering_quality );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_rb );

	// inform in what buffers we draw
	fbo.SetNumOfColorAttachements(1);

	// create the txtrs
	fai.CreateEmpty( r::w * r::rendering_quality, r::h * r::rendering_quality, GL_RGB, GL_RGB );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.GetGLID(), 0 );

	// test if success
	if( !fbo.CheckStatus() )
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
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
}


/*
=======================================================================================================================================
SetScissors                                                                                                                           =
the function sets the scissors and returns the pixels that are in rect region                                                         =
=======================================================================================================================================
*/
/*static int SetScissors( const vec3_t& light_pos_vspace, float radius )
{
	float _w = r::w * r::rendering_quality;
	int rect[4]={ 0,0,_w,_w };
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
			int ix=(int)((fx+1.0f)*_w*0.5f);

			float px=-pz1*nz1/nx1;
			if (px<l.x)
				rect[0]=max(rect[0],ix);
			else
				rect[2]=min(rect[2],ix);
		}

		if (pz2<0)
		{
			float fx=nz2*e1/nx2;
			int ix=(int)((fx+1.0f)*_w*0.5f);

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
	if( (uint)n == _w*r::h )
	{
		glDisable(GL_SCISSOR_TEST);
		return _w*r::h;
	}

	glScissor(rect[0],rect[1],rect[2]-rect[0],rect[3]-rect[1]);
	glEnable(GL_SCISSOR_TEST);

	//INFO( rect[0] << ' ' << rect[1] << ' ' << rect[2]-rect[0] << ' ' << rect[3]-rect[1] );
	return n;
}*/

/*
=======================================================================================================================================
SetStencilMask [point light]                                                                                                          =
clears the stencil buffer and draws a shape in the stencil buffer (in this case the shape is a UV shpere)                             =
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


	vec3_t light_pos_eye_space = light.translation_wspace.Transformed( cam.GetViewMatrix() );

	glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1/light.radius)[0] );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( light.GetDiffuseColor(), 1.0 ))[0] );
	glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( light.GetSpecularColor(), 1.0 ))[0] );

	//** render quad **
	int loc = shader.GetAttributeLocation(0); // view_vector
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableVertexAttribArray( loc );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glVertexAttribPointer( loc, 3, GL_FLOAT, 0, 0, &view_vectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableVertexAttribArray( loc );

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

	//** set texture matrix for shadowmap projection **
	// Bias * P_light * V_light * inv( V_cam )
	const float mBias[] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
	glActiveTexture( GL_TEXTURE0 );
	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( mBias );
	r::MultMatrix( light.camera.GetProjectionMatrix() );
	r::MultMatrix( light.camera.GetViewMatrix() );
	r::MultMatrix( cam.transformation_wspace );
	glMatrixMode(GL_MODELVIEW);

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

	DEBUG_ERR( light.texture == NULL ); // No texture attached to the light

	// set the light texture
	shdr->LocTexUnit( shdr->GetUniformLocation(5), *light.texture, 4 );
	// before we render disable anisotropic in the light.texture because it produces artefacts. ToDo: see if this is unececeary in future drivers
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// the planes
	//glUniform2fv( shdr->GetUniformLocation("planes"), 1, &planes[0] );
	glUniform2fv( shdr->GetUniformLocation(4), 1, &planes[0] );

	// the pos and max influence distance
	vec3_t light_pos_eye_space = light.translation_wspace.Transformed( cam.GetViewMatrix() );
	glLightfv( GL_LIGHT0, GL_POSITION, &vec4_t(light_pos_eye_space, 1.0/light.GetDistance())[0] );

	// the colors
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  &(vec4_t( light.GetDiffuseColor(), 1.0 ))[0] );
	glLightfv( GL_LIGHT0, GL_SPECULAR,  &(vec4_t( light.GetSpecularColor(), 1.0 ))[0] );

	// the shadow stuff
	// render depth to texture and then bind it
	if( light.casts_shadow )
	{
		shdr->LocTexUnit( shdr->GetUniformLocation(6), r::is::shadows::shadow_map, 5 );
	}

	//** render quad **
	int loc = shdr_is_lp_spot_light_nos->GetAttributeLocation(0); // view_vector
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableVertexAttribArray( loc );

	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glVertexAttribPointer( loc, 3, GL_FLOAT, 0, 0, &view_vectors[0] );

	glDrawArrays( GL_QUADS, 0, 4 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableVertexAttribArray( loc );

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
	r::SetViewport( 0, 0, r::w*r::rendering_quality, r::h*r::rendering_quality );

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

			case light_t::SPOT:
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
