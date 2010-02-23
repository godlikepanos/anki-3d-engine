#include "light_props.h"
#include "parser.h"
#include "texture.h"


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool light_props_t::Load( const char* filename )
{
Scanner scanner;
	if( !scanner.loadFile( filename ) ) return false;

	const Scanner::Token* token;

	do
	{
		token = &scanner.getNextToken();

		//** DIFFUSE_COL **
		if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DIFFUSE_COLOR" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 3, &diffuse_col[0] );
		}
		//** SPECULAR_COL **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "SPECULAR_COLOR" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 3, &specular_col[0] );
		}
		//** RADIUS **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "RADIUS" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			radius = (token->type == Scanner::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** CASTS_SHADOW **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "CASTS_SHADOW" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			casts_shadow = token->value.int_;
		}
		//** DISTANCE **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DISTANCE" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			distance = (token->type == Scanner::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** FOV_X **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "FOV_X" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			fov_x = (token->type == Scanner::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** FOV_Y **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "FOV_Y" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}

			fov_y = (token->type == Scanner::DT_FLOAT) ? token->value.float_ : float(token->value.int_);
		}
		//** TEXTURE **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "TEXTURE" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
				
			texture = rsrc::textures.Load( token->value.string );
			texture->TexParameter( GL_TEXTURE_MAX_ANISOTROPY_EXT, 0 );
		}
		// end of file
		else if( token->code == Scanner::TC_EOF )
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
