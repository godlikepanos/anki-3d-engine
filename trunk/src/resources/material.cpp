#include <string.h>
#include "material.h"
#include "resource.h"
#include "Scanner.h"
#include "parser.h"
#include "texture.h"
#include "shader_prog.h"
#include "renderer.h"


#define MTL_ERROR( x ) ERROR( "Material (" << GetPath() << GetName() << "): " << x );


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
	Scanner scanner;
	if( !scanner.loadFile( filename ) ) return false;

	const Scanner::Token* token;

	do
	{
		token = &scanner.getNextToken();

		//** SHADER_PROG **
		if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "SHADER_PROG" ) )
		{
			if( shader_prog ) ERROR( "Shader program allready loaded" );

			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			shader_prog = rsrc::shaders.Load( token->value.string );
		}
		/*//** DEPTH_SHADER_PROG **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DEPTH_SHADER_PROG" ) )
		{
			if( depth.shader_prog ) ERROR( "Depth shader program allready loaded" );

			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			depth.shader_prog = rsrc::shaders.Load( token->value.string );
		}*/
		//** DEPTH_MATERIAL **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DEPTH_PASS_MATERIAL" ) )
		{
			if( dp_mtl ) ERROR( "Depth material already loaded" );

			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			dp_mtl = rsrc::materials.Load( token->value.string );
		}
		//** BLENDS **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "BLENDS" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			blends = token->value.int_;
		}
		//** REFRACTS **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "REFRACTS" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			refracts = token->value.int_;
		}
		//** BLENDING_SFACTOR **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "BLENDING_SFACTOR" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_IDENTIFIER )
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
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "BLENDING_DFACTOR" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_IDENTIFIER )
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
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DEPTH_TESTING" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			depth_testing = token->value.int_;
		}
		//** GRASS_MAP **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "GRASS_MAP" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			grass_map = rsrc::textures.Load( token->value.string );
		}
		//** WIREFRAME **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "WIREFRAME" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			wireframe = token->value.int_;
		}
		//** CASTS_SHADOW **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "CASTS_SHADOW" ) )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
			casts_shadow = token->value.int_;
		}
		//** USER_DEFINED_VARS **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "USER_DEFINED_VARS" ) )
		{
			// read {
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}
			// loop all the vars
			do
			{
				// read var type
				user_defined_var_t::type_e type;
				token = &scanner.getNextToken();
				if( token->code == Scanner::TC_RBRACKET )
					break;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "TEXTURE" ) )
					type = user_defined_var_t::VT_TEXTURE;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "FLOAT" ) )
					type = user_defined_var_t::VT_FLOAT;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "VEC3" ) )
					type = user_defined_var_t::VT_VEC3;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "VEC4" ) )
					type = user_defined_var_t::VT_VEC4;
				else
				{
					PARSE_ERR_EXPECTED( "TEXTURE or FLOAT or VEC3 or VEC4 or }" );
					return false;
				}

				user_defined_vars.push_back( user_defined_var_t() ); // create new var
				user_defined_var_t& var = user_defined_vars.back();
				var.type = type;

				// read the name
				token = &scanner.getNextToken();
				if( token->code == Scanner::TC_IDENTIFIER )
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
					case user_defined_var_t::VT_TEXTURE:
						token = &scanner.getNextToken();
						if( token->code == Scanner::TC_STRING )
						{
							var.value.texture = rsrc::textures.Load( token->value.string );
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "IS_FAI" ) )
						{
							var.value.texture = &r::is::fai;
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "MS_NORMAL_FAI" ) )
						{
							var.value.texture = &r::ms::normal_fai;
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "MS_DEPTH_FAI" ) )
						{
							var.value.texture = &r::ms::depth_fai;
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "PPS_FAI" ) )
						{
							var.value.texture = &r::pps::fai;
						}
						else
						{
							PARSE_ERR_EXPECTED( "string or IS_FAI or MS_NORMAL_FAI or MS_DEPTH_FAI or PPS_FAI" );
							return false;
						}
						break;
					// float
					case user_defined_var_t::VT_FLOAT:
						token = &scanner.getNextToken();
						if( token->code == Scanner::TC_NUMBER && token->type == Scanner::DT_FLOAT )
							var.value.float_ = token->value.float_;
						else
						{
							PARSE_ERR_EXPECTED( "float" );
							return false;
						}
						break;
					// vec2
					case user_defined_var_t::VT_VEC2:
						ERROR( "Unimplemented" );
						break;
					// vec3
					case user_defined_var_t::VT_VEC3:
						if( !ParseArrOfNumbers<float>( scanner, true, true, 3, &var.value.vec3[0] ) ) return false;
						break;
					// vec4
					case user_defined_var_t::VT_VEC4:
						if( !ParseArrOfNumbers<float>( scanner, true, true, 4, &var.value.vec4[0] ) ) return false;
						break;
				};

			}while(true); // end loop for all the vars

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

	return AdditionalInit();
}


//=====================================================================================================================================
// AdditionalInit                                                                                                                     =
//=====================================================================================================================================
bool material_t::AdditionalInit()
{
	// sanity checks
	if( !shader_prog )
	{
		MTL_ERROR( "Without shader is like cake without sugar (missing SHADER_PROG)" );
		return false;
	}

	/*if( !depth.shader_prog )
	{
		MTL_ERROR( "Without depth shader is like cake without shadow (missing DEPTH_SHADER_PROG)" );
		return false;
	}*/

	// for all user defined vars get their location
	shader_prog->Bind();
	for( uint i=0; i<user_defined_vars.size(); i++ )
	{
		int loc = shader_prog->GetUniformLocation( user_defined_vars[i].name.c_str() );
		if( loc == -1 )
		{
			MTL_ERROR( "Shader \"" << shader_prog->GetName() << "\" and user defined var \"" << user_defined_vars[i].name <<
			           "\" do not combine. Incorrect location" );
			return false;
		}
		user_defined_vars[i].uniform_location = loc;
	}
	shader_prog->Unbind();

	// init the attribute locations
	attrib_locs.tanget = shader_prog->GetAttributeLocationSilently( "tangent" );
	attrib_locs.position = shader_prog->GetAttributeLocationSilently( "position" );
	attrib_locs.normal = shader_prog->GetAttributeLocationSilently( "normal" );
	attrib_locs.tex_coords = shader_prog->GetAttributeLocationSilently( "tex_coords" );

	// vertex weights
	attrib_locs.vert_weight_bones_num = shader_prog->GetAttributeLocationSilently( "vert_weight_bones_num" );
	if( attrib_locs.vert_weight_bones_num != -1 )
	{
		attrib_locs.vert_weight_bone_ids = shader_prog->GetAttributeLocation( "vert_weight_bone_ids" );
		attrib_locs.vert_weight_weights = shader_prog->GetAttributeLocation( "vert_weight_weights" );
		uni_locs.skinning_rotations = shader_prog->GetUniformLocation( "skinning_rotations" );
		uni_locs.skinning_translations = shader_prog->GetUniformLocation( "skinning_translations" );
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
	rsrc::shaders.Unload( shader_prog );

	// loop all user defined vars and unload the textures
	for( uint i=0; i<user_defined_vars.size(); i++ )
	{
		if( user_defined_vars[i].type == user_defined_var_t::VT_TEXTURE )
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
	shader_prog = NULL;
	blends = false;
	blending_sfactor = GL_ONE;
	blending_dfactor = GL_ZERO;
	depth_testing = true;
	wireframe = false;
	grass_map = NULL;
	casts_shadow = true;
	refracts = false;
	dp_mtl = NULL;
	/*depth.shader_prog = NULL;
	depth.alpha_testing_map = NULL;*/
}


//=====================================================================================================================================
// Setup                                                                                                                              =
//=====================================================================================================================================
void material_t::Setup()
{
	shader_prog->Bind();

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
			case user_defined_var_t::VT_TEXTURE:
				shader_prog->LocTexUnit( udv->uniform_location, *udv->value.texture, texture_unit++ );
				break;
			// float
			case user_defined_var_t::VT_FLOAT:
				glUniform1f( udv->uniform_location, udv->value.float_ );
				break;
			// vec2
			case user_defined_var_t::VT_VEC2:
				glUniform2fv( udv->uniform_location, 1, &udv->value.vec2[0] );
				break;
			// vec3
			case user_defined_var_t::VT_VEC3:
				glUniform3fv( udv->uniform_location, 1, &udv->value.vec3[0] );
				break;
			// vec4
			case user_defined_var_t::VT_VEC4:
				glUniform4fv( udv->uniform_location, 1, &udv->value.vec4[0] );
				break;
		}
	}
}



