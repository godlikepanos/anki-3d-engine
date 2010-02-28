#include <string.h>
#include "Material.h"
#include "Resource.h"
#include "Scanner.h"
#include "parser.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "Renderer.h"


#define MTL_ERROR( x ) ERROR( "Material (" << getRsrcPath() << getRsrcName() << "): " << x );


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
load                                                                                                                                  =
=======================================================================================================================================
*/
bool Material::load( const char* filename )
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
			if( shaderProg ) ERROR( "Shader program already loaded" );

			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			shaderProg = rsrc::shaders.load( token->value.string );
		}
		/*//** DEPTH_SHADER_PROG **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DEPTH_SHADER_PROG" ) )
		{
			if( depth.shaderProg ) ERROR( "Depth shader program already loaded" );

			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			depth.shaderProg = rsrc::shaders.load( token->value.string );
		}*/
		//** DEPTH_MATERIAL **
		else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "DEPTH_PASS_MATERIAL" ) )
		{
			if( dpMtl ) ERROR( "Depth material already loaded" );

			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			dpMtl = rsrc::materials.load( token->value.string );
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
			blendingSfactor = gl_enum;
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
			blendingDfactor = gl_enum;
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
			depthTesting = token->value.int_;
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
			grassMap = rsrc::textures.load( token->value.string );
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
			castsShadow = token->value.int_;
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
				UserDefinedVar::Type type;
				token = &scanner.getNextToken();
				if( token->code == Scanner::TC_RBRACKET )
					break;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "TEXTURE" ) )
					type = UserDefinedVar::VT_TEXTURE;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "FLOAT" ) )
					type = UserDefinedVar::VT_FLOAT;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "VEC3" ) )
					type = UserDefinedVar::VT_VEC3;
				else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "VEC4" ) )
					type = UserDefinedVar::VT_VEC4;
				else
				{
					PARSE_ERR_EXPECTED( "TEXTURE or FLOAT or VEC3 or VEC4 or }" );
					return false;
				}

				userDefinedVars.push_back( UserDefinedVar() ); // create new var
				UserDefinedVar& var = userDefinedVars.back();
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
					case UserDefinedVar::VT_TEXTURE:
						token = &scanner.getNextToken();
						if( token->code == Scanner::TC_STRING )
						{
							var.value.texture = rsrc::textures.load( token->value.string );
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "IS_FAI" ) )
						{
							var.value.texture = &R::Is::fai;
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "MS_NORMAL_FAI" ) )
						{
							var.value.texture = &R::Ms::normalFai;
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "MS_DEPTH_FAI" ) )
						{
							var.value.texture = &R::Ms::depthFai;
						}
						else if( token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "PPS_FAI" ) )
						{
							var.value.texture = &R::Pps::fai;
						}
						else
						{
							PARSE_ERR_EXPECTED( "string or IS_FAI or MS_NORMAL_FAI or MS_DEPTH_FAI or PPS_FAI" );
							return false;
						}
						break;
					// float
					case UserDefinedVar::VT_FLOAT:
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
					case UserDefinedVar::VT_VEC2:
						ERROR( "Unimplemented" );
						break;
					// vec3
					case UserDefinedVar::VT_VEC3:
						if( !ParseArrOfNumbers<float>( scanner, true, true, 3, &var.value.vec3[0] ) ) return false;
						break;
					// vec4
					case UserDefinedVar::VT_VEC4:
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

	return additionalInit();
}


//=====================================================================================================================================
// additionalInit                                                                                                                     =
//=====================================================================================================================================
bool Material::additionalInit()
{
	// sanity checks
	if( !shaderProg )
	{
		MTL_ERROR( "Without shader is like cake without sugar (missing SHADER_PROG)" );
		return false;
	}

	/*if( !depth.shaderProg )
	{
		MTL_ERROR( "Without depth shader is like cake without shadow (missing DEPTH_SHADER_PROG)" );
		return false;
	}*/

	// for all user defined vars get their location
	shaderProg->bind();
	for( uint i=0; i<userDefinedVars.size(); i++ )
	{
		int loc = shaderProg->getUniLoc( userDefinedVars[i].name.c_str() );
		if( loc == -1 )
		{
			MTL_ERROR( "Shader \"" << shaderProg->getRsrcName() << "\" and user defined var \"" << userDefinedVars[i].name <<
			           "\" do not combine. Incorrect location" );
			return false;
		}
		userDefinedVars[i].uniLoc = loc;
	}
	shaderProg->unbind();

	// init the attribute locations
	attribLocs.tanget = shaderProg->getAttribLocSilently( "tangent" );
	attribLocs.position = shaderProg->getAttribLocSilently( "position" );
	attribLocs.normal = shaderProg->getAttribLocSilently( "normal" );
	attribLocs.texCoords = shaderProg->getAttribLocSilently( "texCoords" );

	// vertex weights
	attribLocs.vertWeightBonesNum = shaderProg->getAttribLocSilently( "vertWeightBonesNum" );
	if( attribLocs.vertWeightBonesNum != -1 )
	{
		attribLocs.vertWeightBoneIds = shaderProg->getAttribLoc( "vertWeightBoneIds" );
		attribLocs.vertWeightWeights = shaderProg->getAttribLoc( "vertWeightWeights" );
		uniLocs.skinningRotations = shaderProg->getUniLoc( "skinningRotations" );
		uniLocs.skinningTranslations = shaderProg->getUniLoc( "skinningTranslations" );
	}

	return true;
}


/*
=======================================================================================================================================
unload                                                                                                                                =
=======================================================================================================================================
*/
void Material::unload()
{
	rsrc::shaders.unload( shaderProg );

	// loop all user defined vars and unload the textures
	for( uint i=0; i<userDefinedVars.size(); i++ )
	{
		if( userDefinedVars[i].type == UserDefinedVar::VT_TEXTURE )
			rsrc::textures.unload( userDefinedVars[i].value.texture );
	}

	// the grass map
	if( grassMap )
		rsrc::textures.unload( grassMap );
}

/*
=======================================================================================================================================
setToDefault                                                                                                                          =
=======================================================================================================================================
*/
void Material::setToDefault()
{
	shaderProg = NULL;
	blends = false;
	blendingSfactor = GL_ONE;
	blendingDfactor = GL_ZERO;
	depthTesting = true;
	wireframe = false;
	grassMap = NULL;
	castsShadow = true;
	refracts = false;
	dpMtl = NULL;
	/*depth.shaderProg = NULL;
	depth.alpha_testing_map = NULL;*/
}


//=====================================================================================================================================
// setup                                                                                                                              =
//=====================================================================================================================================
void Material::setup()
{
	shaderProg->bind();

	if( blends )
	{
		glEnable( GL_BLEND );
		//glDisable( GL_BLEND );
		glBlendFunc( blendingSfactor, blendingDfactor );
	}
	else
		glDisable( GL_BLEND );


	if( depthTesting )  glEnable( GL_DEPTH_TEST );
	else                 glDisable( GL_DEPTH_TEST );

	if( wireframe )  glPolygonMode( GL_FRONT, GL_LINE );
	else             glPolygonMode( GL_FRONT, GL_FILL );


	// now loop all the user defined vars and set them
	uint texture_unit = 0;
	Vec<UserDefinedVar>::iterator udv;
	for( udv=userDefinedVars.begin(); udv!=userDefinedVars.end(); udv++ )
	{
		switch( udv->type )
		{
			// texture
			case UserDefinedVar::VT_TEXTURE:
				shaderProg->locTexUnit( udv->uniLoc, *udv->value.texture, texture_unit++ );
				break;
			// float
			case UserDefinedVar::VT_FLOAT:
				glUniform1f( udv->uniLoc, udv->value.float_ );
				break;
			// vec2
			case UserDefinedVar::VT_VEC2:
				glUniform2fv( udv->uniLoc, 1, &udv->value.vec2[0] );
				break;
			// vec3
			case UserDefinedVar::VT_VEC3:
				glUniform3fv( udv->uniLoc, 1, &udv->value.vec3[0] );
				break;
			// vec4
			case UserDefinedVar::VT_VEC4:
				glUniform4fv( udv->uniLoc, 1, &udv->value.vec4[0] );
				break;
		}
	}
}



