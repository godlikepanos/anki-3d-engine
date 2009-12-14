#include "skybox.h"
#include "resource.h"
#include "renderer.h"
#include "gmath.h"
#include "camera.h"
#include "scene.h"


static float coords [][4][3] =
{
	// front
	{ { 1,  1, -1}, {-1,  1, -1}, {-1, -1, -1}, { 1, -1, -1} },
	// back
	{ {-1,  1,  1}, { 1,  1,  1}, { 1, -1,  1}, {-1, -1,  1} },
	// left
	{ {-1,  1, -1}, {-1,  1,  1}, {-1, -1,  1}, {-1, -1, -1} },
	// right
	{ { 1,  1,  1}, { 1,  1, -1}, { 1, -1, -1}, { 1, -1,  1} },
	// up
	{ { 1,  1,  1}, {-1,  1,  1}, {-1,  1, -1}, { 1,  1, -1} },
	//
	{ { 1, -1, -1}, {-1, -1, -1}, {-1, -1,  1}, { 1, -1,  1} }
};



/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool skybox_t::Load( const char* filenames[6] )
{
	for( int i=0; i<6; i++ )
	{
		textures[i] = rsrc::textures.Load( filenames[i] );
	}

	noise = rsrc::textures.Load( "gfx/noise2.tga" );
	noise->TexParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noise->TexParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );

	shader = rsrc::shaders.Load( "shaders/ms_mp_skybox.shdr" );

	return true;
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void skybox_t::Render( const mat3_t& rotation )
{
	//glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	glPushMatrix();

	shader->Bind();
	glUniform1i( shader->GetUniformLocation("colormap"), 0 );
	shader->LocTexUnit( shader->GetUniformLocation("noisemap"), *noise, 1 );
	glUniform1f( shader->GetUniformLocation("timer"), (rotation_ang/(2*PI))*100 );
	glUniform3fv( shader->GetUniformLocation("scene_ambient_color"), 1, &(vec3_t( 1.0, 1.0, 1.0 ) / scene::GetAmbientColor())[0] );

	// set the rotation matrix
	mat3_t tmp( rotation );
	tmp.RotateYAxis(rotation_ang);
	r::LoadMatrix( mat4_t( tmp ) );
	rotation_ang += 0.0001;
	if( rotation_ang >= 2*PI ) rotation_ang = 0.0;



	const float ffac = 0.001; // fault factor. To eliminate the artefacts in the edge of the quads caused by texture filtering
	float uvs [][2] = { {1.0-ffac,1.0-ffac}, {0.0+ffac,1.0-ffac}, {0.0+ffac,0.0+ffac}, {1.0-ffac,0.0+ffac} };

	for( int i=0; i<6; i++ )
	{
		textures[i]->Bind(0);
		glBegin( GL_QUADS );
			glTexCoord2fv( uvs[0] );
			glVertex3fv( coords[i][0] );
			glTexCoord2fv( uvs[1] );
			glVertex3fv( coords[i][1] );
			glTexCoord2fv( uvs[2] );
			glVertex3fv( coords[i][2] );
			glTexCoord2fv( uvs[3] );
			glVertex3fv( coords[i][3] );
		glEnd();
	}

	glPopMatrix();
}
