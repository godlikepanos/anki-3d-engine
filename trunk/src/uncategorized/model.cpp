#include "model.h"
#include "scanner.h"
#include "parser.h"
#include "mesh.h"


/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool model_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;
	const scanner_t::token_t* token;


	do
	{
		token = &scanner.GetNextToken();

		// I want only strings and yes I prefere G-Strings
		if( token->code == scanner_t::TC_STRING )
		{
			mesh_data_t* mesh = rsrc::mesh_datas.Load( token->value.string );
			if( mesh == NULL )
			{
				ERROR( "Model \"" << filename << "\": Cannot load mesh \"" << token->value.string << "\"" );
				return false;
			}
			meshes.push_back( mesh );
		}
		// end of file
		else if( token->code == scanner_t::TC_EOF )
		{
			break;
		}
		// other crap
		else
		{
			PARSE_ERR_UNEXPECTED();
			return false;
		}

	}while( true );

	return true;
}


/*
=======================================================================================================================================
Unload                                                                                                                                =
=======================================================================================================================================
*/
void model_t::Unload()
{
	for( uint i=0; i<meshes.size(); i++ )
	{
		rsrc::mesh_datas.Unload( meshes[i] );
	}
}
