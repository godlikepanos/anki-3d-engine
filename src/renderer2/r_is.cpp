/*
#include "renderer.hpp"
#include "light.h"
#include "scene.h"

float renderer_t::illumination_stage_t::point_light_pass_t::smo_uvs_coords [] = { -0.000000, 0.000000, -1.000000, 0.500000, 0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.707107, 0.000000, 0.707107, -0.000000, 0.707107, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.000000, 0.707107, -0.707107, 0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.000000, 0.707107, 0.707107, -0.707107, -0.000000, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.000000, -1.000000, -0.707107, -0.000000, -0.707107, -0.500000, 0.500000, -0.707107, -0.000000, 0.000000, -1.000000, -0.500000, -0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, -0.707107, -0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, 0.000000, 1.000000, -0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, -0.500000, -0.500000, -0.707107, -0.000000, 0.000000, -1.000000, 0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, 0.707107, 0.000000, 0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, 0.000000, 1.000000, 0.500000, -0.500000, 0.707107, -0.000000, 0.000000, -1.000000, 0.707107, 0.000000, -0.707107, 0.500000, -0.500000, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, 0.000000, -0.707107, 1.000000, 0.000000, -0.000000, 0.500000, -0.500000, -0.707107, 1.000000, 0.000000, -0.000000, 0.707107, -0.707107, 0.000000, 0.707107, -0.707107, 0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.000000, 0.707107, 0.707107, -0.707107, 0.000000, 0.707107, 0.000000, 0.707107, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.707107, -0.707107, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -1.000000, 0.000000, 0.500000, -0.500000, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, -0.707107, 0.500000, -0.500000, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, -0.500000, -0.500000, -0.707107, 0.000000, -0.707107, -0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -1.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.707107, -0.707107, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, -0.707107, 0.707107, -0.707107, -0.707107, 0.000000, 0.000000, -0.707107, 0.707107, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.707107, -0.707107, 0.000000, -0.500000, -0.500000, 0.707107, -1.000000, -0.000000, 0.000000, -0.500000, -0.500000, 0.707107, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, -0.707107, -0.500000, -0.500000, -0.707107, -0.707107, -0.707107, 0.000000, -0.707107, -0.000000, -0.707107, -0.707107, -0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -0.707107, -0.000000, -0.707107, -1.000000, -0.000000, 0.000000, -0.500000, 0.500000, -0.707107, -1.000000, -0.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.707107, 0.707107, 0.000000, -1.000000, -0.000000, 0.000000, -0.707107, -0.000000, 0.707107, -0.707107, 0.707107, 0.000000, -0.707107, -0.000000, 0.707107, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.707107, 0.707107, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 1.000000, 0.000000, -0.500000, 0.500000, 0.707107, -0.000000, 0.707107, 0.707107, -0.000000, 0.707107, -0.707107, -0.500000, 0.500000, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 0.707107, -0.707107, -0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 0.707107, -0.707107, -0.000000, 1.000000, 0.000000, 0.500000, 0.500000, -0.707107, -0.000000, 1.000000, 0.000000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.000000, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.707107, 0.707107, 0.000000, -0.000000, 0.707107, 0.707107, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.707107, 0.707107, 0.000000, 0.500000, 0.500000, 0.707107, 1.000000, 0.000000, -0.000000, 0.500000, 0.500000, 0.707107, 0.707107, 0.000000, 0.707107, 0.707107, 0.000000, -0.707107, 0.500000, 0.500000, -0.707107, 0.707107, 0.707107, 0.000000, 0.707107, 0.000000, -0.707107, 0.707107, 0.707107, 0.000000, 1.000000, 0.000000, -0.000000 };
uint renderer_t::illumination_stage_t::point_light_pass_t::smo_uvs_vbo_id = NULL; // stencil masking opt uv sphere vertex buffer object id


//=====================================================================================================================================
// Stencil Masking Opt Uv Sphere                                                                                                      =
//=====================================================================================================================================

// init stencil masking optimization UV sphere
void renderer_t::illumination_stage_t::point_light_pass_t::InitSMOUVS()
{
	glGenBuffers( 1, &smo_uvs_vbo_id );
	glBindBuffer( GL_ARRAY_BUFFER, smo_uvs_vbo_id );
	glBufferData( GL_ARRAY_BUFFER, sizeof(smo_uvs_coords), smo_uvs_coords, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void renderer_t::illumination_stage_t::point_light_pass_t::DrawSMOUVS( const point_light_t& light )
{
	const float scale = 1.2;
	renderer.matrices.model = mat4_t( light.translation_wspace, mat3_t::GetIdentity(), light.radius*scale );

	shader_progs.smouvs.Bind();

	renderer.UpdateMatrices();

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
/// calc the view vector that we will use inside the shader to calculate the frag pos in view space
void renderer_t::illumination_stage_t::CalcViewVector()
{
	int _w = renderer.width;
	int _h = renderer.height;
	int pixels[4][2]={ {_w,_h}, {0,_h}, { 0,0 }, {_w,0} }; // from righ up and CC wise to right down, Just like we render the quad
	int viewport[4]={ 0, 0, _w, _h };

	for( int i=0; i<4; i++ )
	{
		 Original Code:
		r::UnProject( pixels[i][0], pixels[i][1], 10, cam.GetViewMatrix(), cam.GetProjectionMatrix(), viewport,
		              view_vectors[i].x, view_vectors[i].y, view_vectors[i].z );
		view_vectors[i] = cam.GetViewMatrix() * view_vectors[i];
		The original code is the above 3 lines. The optimized follows:

		vec3_t vec;
		vec.x = (2.0*(pixels[i][0]-viewport[0]))/viewport[2] - 1.0;
		vec.y = (2.0*(pixels[i][1]-viewport[1]))/viewport[3] - 1.0;
		vec.z = 1.0;

		view_vectors[i] = vec.GetTransformed( renderer.camera->GetInvProjectionMatrix() );
		// end of optimized code
	}
}


//=====================================================================================================================================
// CalcPlanes                                                                                                                         =
//=====================================================================================================================================
/// calc the planes that we will use inside the shader to calculate the frag pos in view space
void renderer_t::illumination_stage_t::CalcPlanes()
{
	const float& zfar = renderer.camera->GetZFar();
	const float& znear = renderer.camera->GetZNear();
	planes.x = -zfar / (zfar - znear);
	planes.y = -zfar * znear / (zfar - znear);
}


//=====================================================================================================================================
// Init ambient pass                                                                                                                  =
//=====================================================================================================================================
void renderer_t::illumination_stage_t::ambient_pass_t::Init()
{
	shader_prog.CustomLoad( "shaders/is_ap.glsl", "" );
}


//=====================================================================================================================================
// Run ambient pass                                                                                                                   =
//=====================================================================================================================================
void renderer_t::illumination_stage_t::ambient_pass_t::Run() const
{
	glDisable( GL_BLEND );

	// set the shader
	shader_prog.Bind();

	// set the uniforms
	glUniform3fv( shader_prog.GetUniformLocation(0), 1, &((vec3_t)scene::GetAmbientColor())[0] );
	shader_prog.LocTexUnit( shader_prog.GetUniformLocation(1), renderer.ms.fais.diffuse, 0 );

	// Draw quad
	renderer.DrawQuad();
}


//=====================================================================================================================================
// SetStencilMask [point light]                                                                                                       =
//=====================================================================================================================================
/// clears the stencil buffer and draws a shape in the stencil buffer (in this case the shape is a UV shpere)
void renderer_t::illumination_stage_t::point_light_pass_t::SetStencilMask( const point_light_t& light ) const
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
*/
