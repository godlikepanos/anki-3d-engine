#include "light_props.h"
#include "parser.h"
#include "texture.h"


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool light_props_t::Load( const char* filename )
{
scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;

	do
	{
		token = &scanner.GetNextToken();

		//** DIFFUSE_COL **
		if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "DIFFUSE_COLOR" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 3, &diffuse_col[0] );
		}
		//** SPECULAR_COL **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SPECULAR_COLOR" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 3, &specular_col[0] );
		}
		//** RADIUS **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "RADIUS" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			radius = (token->type == scanner_t::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** CASTS_SHADOW **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "CASTS_SHADOW" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			casts_shadow = token->value.int_;
		}
		//** DISTANCE **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "DISTANCE" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			distance = (token->type == scanner_t::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** FOV_X **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "FOV_X" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			fov_x = (token->type == scanner_t::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** FOV_Y **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "FOV_Y" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			fov_y = (token->type == scanner_t::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
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
void light_props_t::Unload()
{
	if( texture != NULL )
		rsrc::textures.Unload( texture );
}
