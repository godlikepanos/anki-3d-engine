#include "material.h"
#include "assets.h"


material_t* material_t::used_previously = NULL;

/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool material_t::Load( const char* filename )
{
	fstream file( filename, ios::in );
	char str [256];

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return false;
	}

	// the textures
	file >> str >> textures_num;
	for( uint i=0; i<textures_num; i++ )
	{
		file >> str >> str;
		textures[i] = ass::LoadTxtr( str );
	}

	// shader
	file >> str >> str;
	shader = ass::LoadShdr( str );

	// colors
	for( uint i=0; i<COLS_NUM; i++ )
	{
		file >> str;
		for( uint j=0; j<4; j++ )
			file >> colors[i][j];
	}

	// shininess
	file >> str >> shininess;

	file.close();
	return true;
}


/*
=======================================================================================================================================
Use                                                                                                                                   =
=======================================================================================================================================
*/
void material_t::Use()
{
	shader->Bind();

//	if( used_previously == this ) return;
//	used_previously = this;

	glMaterialfv( GL_FRONT, GL_AMBIENT, &colors[AMBIENT_COL][0] );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, &colors[DIFFUSE_COL][0] );
	glMaterialfv( GL_FRONT, GL_SPECULAR, &colors[SPECULAR_COL][0] );
	glMaterialf( GL_FRONT, GL_SHININESS, shininess );


	switch( textures_num )
	{
		case 4:
			glUniform1i( shader->GetUniLocation( "t3" ), 3 );
			glActiveTexture( GL_TEXTURE0 + 3 );
			glBindTexture( GL_TEXTURE_2D, textures[3]->gl_id );
		case 3:
			glUniform1i( shader->GetUniLocation( "t2" ), 2 );
			glActiveTexture( GL_TEXTURE0 + 2 );
			glBindTexture( GL_TEXTURE_2D, textures[2]->gl_id );
		case 2:
			glUniform1i( shader->GetUniLocation( "t1" ), 1 );
			glActiveTexture( GL_TEXTURE0 + 1 );
			glBindTexture( GL_TEXTURE_2D, textures[1]->gl_id );
		case 1:
			glUniform1i( shader->GetUniLocation( "t0" ), 0 );
			glActiveTexture( GL_TEXTURE0 + 0 );
			glBindTexture( GL_TEXTURE_2D, textures[0]->gl_id );
	}

}




