#include "light_mtl.h"
#include "parser.h"
#include "texture.h"


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool light_mtl_t::Load( const char* filename )
{
scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;

	do
	{
		token = &scanner.GetNextToken();

		//** DIFFUSE_COL **
		if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "DIFFUSE_COL" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 3, &diffuse_col[0] );
		}
		//** SPECULAR_COL **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SPECULAR_COL" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 3, &specular_col[0] );
		}
		//** TEXTURE **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "TEXTURE" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
				
			texture = rsrc::textures.Load( token->value.string );
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


//=====================================================================================================================================
// Unload                                                                                                                             =
//=====================================================================================================================================
void light_mtl_t::Unload()
{
	if( texture != NULL )
		rsrc::textures.Unload( texture );
}
