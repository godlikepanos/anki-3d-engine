#include <string.h>
#include "material.h"
#include "resource.h"
#include "scanner.h"
#include "parser.h"
#include "texture.h"
#include "shaders.h"
#include "renderer.h"

/*
=======================================================================================================================================
Blending dtuff                                                                                                                        =
=======================================================================================================================================
*/
struct blend_param_t
{
	int gl_enum;
	const char* str;
};

static blend_param_t bparams [] =
{
	{GL_ZERO, "GL_ZERO"},
	{GL_ONE, "GL_ONE"},
	{GL_DST_COLOR, "GL_DST_COLOR"},
	{GL_ONE_MINUS_DST_COLOR, "GL_ONE_MINUS_DST_COLOR"},
	{GL_SRC_ALPHA, "GL_SRC_ALPHA"},
	{GL_ONE_MINUS_SRC_ALPHA, "GL_ONE_MINUS_SRC_ALPHA"},
	{GL_DST_ALPHA, "GL_DST_ALPHA"},
	{GL_ONE_MINUS_DST_ALPHA, "GL_ONE_MINUS_DST_ALPHA"},
	{GL_SRC_ALPHA_SATURATE, "GL_SRC_ALPHA_SATURATE"},
	{GL_SRC_COLOR, "GL_SRC_COLOR"},
	{GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR"}
};

const int BLEND_PARAMS_NUM = 11;

static bool SearchBlendEnum( const char* str, int& gl_enum )
{
	for( int i=0; i<BLEND_PARAMS_NUM; i++ )
	{
		if( !strcmp( bparams[i].str, str) )
		{
			gl_enum = bparams[i].gl_enum;
			return true;
		}
	}
	return false;
}


/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool material_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;

	do
	{
		token = &scanner.GetNextToken();

		//** SHADER **
		if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SHADER" ) )
		{
			if( shader ) ERROR( "Shader allready loaded" );

			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			shader = rsrc::shaders.Load( token->value.string );
		}
		//** DIFFUSE_COL **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "DIFFUSE_COL" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 4, &diffuse_color[0] );
		}
		//** SPECULAR_COL **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SPECULAR_COL" ) )
		{
			ParseArrOfNumbers<float>( scanner, true, true, 4, &specular_color[0] );
		}
		//** SHININESS **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SHININESS" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			if( token->type == scanner_t::DT_FLOAT )
				shininess = token->value.float_;
			else
				shininess = (float)token->value.int_;
		}
		//** BLENDS **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "BLENDS" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			blends = token->value.int_;
		}
		//** BLENDING_SFACTOR **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "BLENDING_SFACTOR" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_IDENTIFIER )
			{
				PARSE_ERR_EXPECTED( "identifier" );
				return false;
			}
			int gl_enum;
			if( !SearchBlendEnum(token->value.string, gl_enum) )
			{
				PARSE_ERR( "Incorrect blending factor \"" << token->value.string << "\"" );
				return false;
			}
			blending_sfactor = gl_enum;
		}
		//** BLENDING_DFACTOR **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "BLENDING_DFACTOR" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_IDENTIFIER )
			{
				PARSE_ERR_EXPECTED( "identifier" );
				return false;
			}
			int gl_enum;
			if( !SearchBlendEnum(token->value.string, gl_enum) )
			{
				PARSE_ERR( "Incorrect blending factor \"" << token->value.string << "\"" );
				return false;
			}
			blending_dfactor = gl_enum;
		}
		//** DEPTH_TESTING **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "DEPTH_TESTING" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			depth_testing = token->value.int_;
		}
		//** GRASS_MAP **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "GRASS_MAP" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			grass_map = rsrc::textures.Load( token->value.string );
		}
		//** WIREFRAME **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "WIREFRAME" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			wireframe = token->value.int_;
		}
		//** CASTS_SHADOW **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "CASTS_SHADOW" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			casts_shadow = token->value.int_;
		}
		//** USER_DEFINED_VARS **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "USER_DEFINED_VARS" ) )
		{
			// read {
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}
			// loop all the vars
			do
			{
				// read var type
				user_defined_var_t::type_e type;
				token = &scanner.GetNextToken();
				if( token->code == scanner_t::TC_RBRACKET )
					break;
				else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "TEXTURE" ) )
					type = user_defined_var_t::TEXTURE;
				else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "FLOAT" ) )
					type = user_defined_var_t::FLOAT;
				else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "VEC3" ) )
					type = user_defined_var_t::VEC3;
				else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "VEC4" ) )
					type = user_defined_var_t::VEC4;
				else
				{
					PARSE_ERR_EXPECTED( "TEXTURE or FLOAT or VEC3 or VEC4 or }" );
					return false;
				}

				user_defined_vars.push_back( user_defined_var_t() ); // create new var
				user_defined_var_t& var = user_defined_vars.back();
				var.type = type;

				// read the name
				token = &scanner.GetNextToken();
				if( token->code == scanner_t::TC_IDENTIFIER )
					var.name = token->value.string;
				else
				{
					PARSE_ERR_EXPECTED( "identifier" );
					return false;
				}

				// read the values
				switch( type )
				{
					// texture
					case user_defined_var_t::TEXTURE:
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_STRING )
						{
							var.value.texture = rsrc::textures.Load( token->value.string );
						}
						else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "IS_FAI" ) )
						{
							var.value.texture = &r::is::fai;
						}
						else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "MS_NORMAL_FAI" ) )
						{
							var.value.texture = &r::ms::normal_fai;
						}
						else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "MS_DEPTH_FAI" ) )
						{
							var.value.texture = &r::ms::depth_fai;
						}
						else
						{
							PARSE_ERR_EXPECTED( "string" );
							return false;
						}
						break;
					// float
					case user_defined_var_t::FLOAT:
						if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_FLOAT )
							var.value.float_ = token->value.float_;
						else
						{
							PARSE_ERR_EXPECTED( "float" );
							return false;
						}
						break;
					// vec3
					case user_defined_var_t::VEC3:
						if( !ParseArrOfNumbers<float>( scanner, true, true, 3, &var.value.vec3[0] ) ) return false;
						break;
					// vec4
					case user_defined_var_t::VEC4:
						if( !ParseArrOfNumbers<float>( scanner, true, true, 4, &var.value.vec4[0] ) ) return false;
						break;
				};

			}while(true); // end loop for all the vars

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

	return InitTheOther();
}


//=====================================================================================================================================
// InitTheOther                                                                                                                       =
//=====================================================================================================================================
bool material_t::InitTheOther()
{
	// sanity check
	if( !shader )
	{
		ERROR( "Material file (\"" << GetPath() << GetName() << "\") without shader is like cake without sugar" );
		return false;
	}

	// for all user defined vars get their location
	shader->Bind();
	for( uint i=0; i<user_defined_vars.size(); i++ )
	{
		int loc = shader->GetUniformLocation( user_defined_vars[i].name.c_str() );
		if( loc == -1 )
		{
			ERROR( "Material \"" << GetPath() << GetName() << "\", shader \"" << shader->GetName() << "\" and user defined var \"" << user_defined_vars[i].name <<
			       "\" do not combine. Incorect location" );
			return false;
		}
		user_defined_vars[i].uniform_location = loc;
	}
	shader->Unbind();

	// get the tangents_attrib_loc
	tangents_attrib_loc = shader->GetAttributeLocationSilently( "tangent" );
	needs_tangents = ( tangents_attrib_loc == -1 ) ? false : true;

	// vertex weights
	vert_weight_bones_num_attrib_loc = shader->GetAttributeLocationSilently( "vert_weight_bones_num" );
	if( vert_weight_bones_num_attrib_loc == -1 )
	{
		needs_vert_weights = false;
	}
	else
	{
		vert_weight_bone_ids_attrib_loc = shader->GetAttributeLocation( "vert_weight_bone_ids" );
		vert_weight_weights_attrib_loc = shader->GetAttributeLocation( "vert_weight_weights" );
		skinning_rotations_uni_loc = shader->GetUniformLocation( "skinning_rotations" );
		skinning_translations_uni_loc = shader->GetUniformLocation( "skinning_translations" );
		needs_vert_weights = true;
	}

	return true;
}


/*
=======================================================================================================================================
Unload                                                                                                                                =
=======================================================================================================================================
*/
void material_t::Unload()
{
	rsrc::shaders.Unload( shader );

	// loop all user defined vars and unload the textures
	for( uint i=0; i<user_defined_vars.size(); i++ )
	{
		if( user_defined_vars[i].type == user_defined_var_t::TEXTURE )
			rsrc::textures.Unload( user_defined_vars[i].value.texture );
	}

	// the grass map
	if( grass_map )
		rsrc::textures.Unload( grass_map );
}

/*
=======================================================================================================================================
SetToDefault                                                                                                                          =
=======================================================================================================================================
*/
void material_t::SetToDefault()
{
	shader = NULL;
	blends = false;
	blending_sfactor = GL_ONE;
	blending_dfactor = GL_ZERO;
	depth_testing = true;
	wireframe = false;
	specular_color = vec4_t(1.0);
	diffuse_color = vec4_t(1.0);
	shininess = 0.0;
	grass_map = NULL;
	casts_shadow = true;
	needs_tangents = true;
	refracts = false;
}


/*
=======================================================================================================================================
Setup                                                                                                                                 =
=======================================================================================================================================
*/
void material_t::Setup()
{
	shader->Bind();

	// first set the standard vars
	glMaterialfv( GL_FRONT, GL_DIFFUSE, &diffuse_color[0] );
	glMaterialfv( GL_FRONT, GL_SPECULAR, &specular_color[0] );
	glMaterialf( GL_FRONT, GL_SHININESS, shininess );

	if( blends )
	{
		glEnable( GL_BLEND );
		//glDisable( GL_BLEND );
		glBlendFunc( blending_sfactor, blending_dfactor );
	}
	else
		glDisable( GL_BLEND );


	if( depth_testing )  glEnable( GL_DEPTH_TEST );
	else                 glDisable( GL_DEPTH_TEST );

	if( wireframe )  glPolygonMode( GL_FRONT, GL_LINE );
	else             glPolygonMode( GL_FRONT, GL_FILL );


	// now loop all the user defined vars and set them
	uint texture_unit = 0;
	vec_t<user_defined_var_t>::iterator udv;
	for( udv=user_defined_vars.begin(); udv!=user_defined_vars.end(); udv++ )
	{
		switch( udv->type )
		{
			// texture
			case user_defined_var_t::TEXTURE:
				shader->LocTexUnit( udv->uniform_location, *udv->value.texture, texture_unit++ );
				break;
			// float
			case user_defined_var_t::FLOAT:
				glUniform1f( udv->uniform_location, udv->value.float_ );
				break;
			// vec3
			case user_defined_var_t::VEC3:
				glUniform3fv( udv->uniform_location, 1, &udv->value.vec3[0] );
				break;
			// vec4
			case user_defined_var_t::VEC4:
				glUniform4fv( udv->uniform_location, 1, &udv->value.vec4[0] );
				break;
		}
	}
}



