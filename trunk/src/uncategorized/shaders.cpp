#include <iostream>
#include <string.h>
#include "shaders.h"
#include "scanner.h"
#include "renderer.h"
#include "texture.h"
#include "parser.h"

#define SHADER_ERROR( x ) ERROR( "Shader prog \"" << GetName() << "\": " << x )
#define SHADER_WARNING( x ) WARNING( "Shader prog \"" << GetName() << "\": " << x )


/*
=======================================================================================================================================
CreateAndCompileShader                                                                                                                =
=======================================================================================================================================
*/
uint shader_prog_t::CreateAndCompileShader( const char* source_code, int type, const char* preprocessor_str, const char* filename )
{
	uint gl_id = 0;
	const char* source_strs[2] = {NULL, NULL};

	// create the shader
	gl_id = glCreateShader( type );

	// attach the source
	source_strs[1] = source_code;
	source_strs[0] = preprocessor_str;

	// compile
	glShaderSource( gl_id, 2, source_strs, NULL );
	glCompileShader( gl_id );

	int success;
	glGetShaderiv( gl_id, GL_COMPILE_STATUS, &success );

	if( !success )
	{
		// print info log
		int info_len = 0;
		int chars_written = 0;
		char* info_log = NULL;

		glGetShaderiv( gl_id, GL_INFO_LOG_LENGTH, &info_len );
		info_log = (char*)malloc( (info_len+1)*sizeof(char) );
		glGetShaderInfoLog( gl_id, info_len, &chars_written, info_log );
		SHADER_ERROR( "Shader: \"" << CutPath(filename) << "\" compile log follows:\n" << info_log );
		free( info_log );
		return 0;
	}

	return gl_id;
}


/*
=======================================================================================================================================
Link                                                                                                                                  =
=======================================================================================================================================
*/
bool shader_prog_t::Link()
{
	// link
	glLinkProgram( gl_id );

	// check if linked correctly
	int success;
	glGetProgramiv( gl_id, GL_LINK_STATUS, &success );

	if( !success )
	{
		int info_len = 0;
		int chars_written = 0;
		char* info_log_txt = NULL;

		glGetProgramiv( gl_id, GL_INFO_LOG_LENGTH, &info_len );

		info_log_txt = (char*)malloc( (info_len+1)*sizeof(char) );
		glGetProgramInfoLog( gl_id, info_len, &chars_written, info_log_txt );
		SHADER_ERROR( "Link log follows:\n" << info_log_txt );
		free( info_log_txt );
		return false;
	}

	return true;
}


/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool shader_prog_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;
	const scanner_t::token_t* token;

	string vert_fname = "", frag_fname = "", pproc = "";
	string vert_inlcuded_code = "";
	string frag_inlcuded_code = "";
	map<string,int> strToUniLoc, strToAttribLoc; // the location name and the location id I want it to have

	do
	{
		token = &scanner.GetNextToken();

		//** vert shader **
		if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "VERTEX_SHADER" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			vert_fname = token->value.string;
		}
		//** frag shader **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "FRAGMENT_SHADER" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			frag_fname = token->value.string;
		}
		//** preprocessor defines **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "PREPROCESSOR_DEFINES" ) )
		{
			token = &scanner.GetNextToken();
			// {
			if( token->code != scanner_t::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			// the defines as strings
			do
			{
				token = &scanner.GetNextToken();

				if( token->code == scanner_t::TC_RBRACKET )
					break;
				else if( token->code == scanner_t::TC_STRING )
				{
					pproc += token->value.string;
					pproc += '\n';
				}
				else
				{
					PARSE_ERR_EXPECTED( "string" );
					return false;
				}

			}while( true );
		}
		//** UNIFORMS **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "UNIFORMS" ) )
		{
			token = &scanner.GetNextToken();
			// {
			if( token->code != scanner_t::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			do
			{
				string name;
				int loc;
				token = &scanner.GetNextToken();

				if( token->code == scanner_t::TC_RBRACKET )
				{
					break;
				}
				else if( token->code == scanner_t::TC_IDENTIFIER )
				{
					name = token->as_string;
					token = &scanner.GetNextToken();
					if( token->code == scanner_t::TC_ASSIGN )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_INT )
						{
							loc = token->value.int_;
						}
						else
						{
							PARSE_ERR_EXPECTED( "integer" );
							return false;
						}
					}
					else
					{
						PARSE_ERR_EXPECTED( "=" );
						return false;
					}
				}
				else
				{
					PARSE_ERR_EXPECTED( "identifier" );
					return false;
				}

				strToUniLoc.insert( make_pair( name, loc ) );
			}while( true );
		}
		//** ATTRIBUTES **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "ATTRIBUTES" ) )
		{
			token = &scanner.GetNextToken();
			// {
			if( token->code != scanner_t::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			do
			{
				string name;
				int loc;
				token = &scanner.GetNextToken();

				if( token->code == scanner_t::TC_RBRACKET )
				{
					break;
				}
				else if( token->code == scanner_t::TC_IDENTIFIER )
				{
					name = token->as_string;
					token = &scanner.GetNextToken();
					if( token->code == scanner_t::TC_ASSIGN )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_INT )
						{
							loc = token->value.int_;
						}
						else
						{
							PARSE_ERR_EXPECTED( "integer" );
							return false;
						}
					}
					else
					{
						PARSE_ERR_EXPECTED( "=" );
						return false;
					}
				}
				else
				{
					PARSE_ERR_EXPECTED( "identifier" );
					return false;
				}

				strToAttribLoc.insert( make_pair( name, loc ) );
			}while( true );
		}
		// VERTEX_SHADER_INCLUDES
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "VERTEX_SHADER_INCLUDES" ) )
		{
			token = &scanner.GetNextToken();
			// {
			if( token->code != scanner_t::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			// for all includes
			do
			{
				token = &scanner.GetNextToken();

				if( token->code == scanner_t::TC_RBRACKET )
					break;
				else if( token->code == scanner_t::TC_STRING )
				{
					string code = ReadFile( token->value.string );
					if( code.length() < 1 )
					{
						SHADER_ERROR( "Cannot include file \"" << token->value.string << "\"" );
						return false;
					}
					vert_inlcuded_code += code;
					vert_inlcuded_code += "\n";
				}
				else
				{
					PARSE_ERR_EXPECTED( "string" );
					return false;
				}

			}while( true );
		}
		// FRAGMENT_SHADER_INCLUDES
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "FRAGMENT_SHADER_INCLUDES" ) )
		{
			token = &scanner.GetNextToken();
			// {
			if( token->code != scanner_t::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			// for all includes
			do
			{
				token = &scanner.GetNextToken();

				if( token->code == scanner_t::TC_RBRACKET )
					break;
				else if( token->code == scanner_t::TC_STRING )
				{
					string code = ReadFile( token->value.string );
					if( code.length() < 1 )
					{
						SHADER_ERROR( "Cannot include file \"" << token->value.string << "\"" );
						return false;
					}
					frag_inlcuded_code += code;
					frag_inlcuded_code += "\n";
				}
				else
				{
					PARSE_ERR_EXPECTED( "string" );
					return false;
				}

			}while( true );
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

	// a few sanity checks
	if( vert_fname == "" || frag_fname == "" )
	{
		SHADER_ERROR( "Forgot to put vertex or fragment shader" );
		return false;
	}

	// add the standard preprocessor defines from renderer. Note that these defines are defined prior the shader code
	string extra_code = r::GetStdShaderPreprocDefines() + pproc;

	// load, compile and link
	string vert_source = ReadFile( vert_fname.c_str() );
	if( vert_source.length() < 1 )
	{
		SHADER_ERROR( "Cannot read file \"" << vert_fname << "\"" );
		return false;
	}
	string frag_source = ReadFile( frag_fname.c_str() );
	if( frag_source.length() < 1 )
	{
		SHADER_ERROR( "Cannot read file \"" << frag_source << "\"" );
		return false;
	}

	uint vert_gl_id = CreateAndCompileShader( vert_source.c_str(), GL_VERTEX_SHADER, (extra_code+vert_inlcuded_code).c_str(), vert_fname.c_str() );
	if( vert_gl_id == 0 ) return false;

	uint frag_gl_id = CreateAndCompileShader( frag_source.c_str(), GL_FRAGMENT_SHADER, (extra_code+frag_inlcuded_code).c_str(), frag_fname.c_str() );
	if( frag_gl_id == 0 ) return false;

	gl_id = glCreateProgram();
	glAttachShader( gl_id, vert_gl_id );
	glAttachShader( gl_id, frag_gl_id );

	if( !Link() ) return false;


	FillTheLocationsVectors( strToUniLoc, strToAttribLoc );
	GetUniAndAttribLocs();

	// done
	Unbind();
	return true;
}


//=====================================================================================================================================
// FillTheLocationsVectors                                                                                                            =
//=====================================================================================================================================
bool shader_prog_t::FillTheLocationsVectors( map<string,int>& strToUniLoc, map<string,int>& strToAttribLoc )
{
	int max = 0;
	map<string,int>::iterator it;

	Bind();

	// uniforms
	for( it=strToUniLoc.begin(); it!=strToUniLoc.end(); it++ )
	{
		if( it->second > max ) max = it->second;
	}
	custom_uni_loc_to_real_loc.assign( max + 1, -1 );

	for( it=strToUniLoc.begin(); it!=strToUniLoc.end(); it++ )
	{
		if( custom_uni_loc_to_real_loc[ it->second ] != -1 )
		{
			SHADER_ERROR( "The uniform \"" << it->first << "\" has the same value with another one" );
			return false;
		}
		int loc = GetUniformLocation( (it->first).c_str() );
		if( loc == -1 )
		{
			SHADER_ERROR( "Check the previous error" );
			return false;
		}
		custom_uni_loc_to_real_loc[it->second] = loc;
	}


	// attributes
	max = 0;

	for( it=strToAttribLoc.begin(); it!=strToAttribLoc.end(); it++ )
	{
		if( it->second > max ) max = it->second;
	}
	custom_attrib_loc_to_real_loc.assign( max + 1, -1 );

	for( it=strToAttribLoc.begin(); it!=strToAttribLoc.end(); it++ )
	{
		if( custom_attrib_loc_to_real_loc[ it->second ] != -1 )
		{
			SHADER_ERROR( "The attribute \"" << it->first << "\" has the same value with another one" );
			return false;
		}
		int loc = GetAttributeLocation( (it->first).c_str() );
		if( loc == -1 )
		{
			SHADER_ERROR( "Check the previous error" );
			return false;
		}
		custom_attrib_loc_to_real_loc[it->second] = loc;
	}

	Unbind();
	return true;
}

//=====================================================================================================================================
// GetUniAndAttribLocs                                                                                                                =
//=====================================================================================================================================
void shader_prog_t::GetUniAndAttribLocs()
{
	/*int num;
	char name_[256];
	GLsizei length;
	GLint size;
	GLenum type;

	// attrib locations
	glGetProgramiv( gl_id, GL_ACTIVE_ATTRIBUTES, &num );
	for( int i=0; i<num; i++ )
	{
		glGetActiveAttrib( gl_id, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_ );
		name_[ length ] = '\0';

		// check if its FFP location
		if( GetAttributeLocationSilently(name_) == -1 )
		{
			//SHADER_WARNING( "You are using FFP vertex attributes (\"" << name_ << "\")" );
			continue;
		}

		custom_attrib_loc_to_real_loc.push_back( loc_t( name_, i ) );
	}


	// uni locations
	glGetProgramiv( gl_id, GL_ACTIVE_UNIFORMS, &num );
	for( int i=0; i<num; i++ )
	{
		glGetActiveUniform( gl_id, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_ );
		name_[ length ] = '\0';

		// check if its FFP location
		if( GetUniformLocationSilently(name_) == -1 )
		{
			//SHADER_WARNING( "You are using FFP vertex attributes (\"" << name_ << "\")" );
			continue;
		}

		custom_uni_loc_to_real_loc.push_back( loc_t( name_, i ) );
	}*/

}

/*
=======================================================================================================================================
GetUniformLocation                                                                                                                    =
=======================================================================================================================================
*/
int shader_prog_t::GetUniformLocation( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 );
	int loc = glGetUniformLocation( gl_id, name );
	if( loc == -1 ) SHADER_ERROR( "Cannot get uniform loc \"" << name << '\"' );
	return loc;
}


/*
=======================================================================================================================================
GetAttributeLocation                                                                                                                  =
=======================================================================================================================================
*/
int shader_prog_t::GetAttributeLocation( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 );
	int loc = glGetAttribLocation( gl_id, name );
	if( loc == -1 ) SHADER_ERROR( "Cannot get attrib location \"" << name << '\"' );
	return loc;
}


//=====================================================================================================================================
// GetUniformLocationSilently                                                                                                         =
//=====================================================================================================================================
int shader_prog_t::GetUniformLocationSilently( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 );
	return glGetUniformLocation( gl_id, name );
}


//=====================================================================================================================================
// GetAttributeLocationSilently                                                                                                       =
//=====================================================================================================================================
int shader_prog_t::GetAttributeLocationSilently( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 );
	return glGetAttribLocation( gl_id, name );
}


//=====================================================================================================================================
// GetUniformLocation                                                                                                                 =
//=====================================================================================================================================
int shader_prog_t::GetUniformLocation( int id ) const
{
	DEBUG_ERR( uint(id) >= custom_uni_loc_to_real_loc.size() );
	DEBUG_ERR( custom_uni_loc_to_real_loc[id] == -1 );
	return custom_uni_loc_to_real_loc[id];
}


//=====================================================================================================================================
// GetAttributeLocation                                                                                                               =
//=====================================================================================================================================
int shader_prog_t::GetAttributeLocation( int id ) const
{
	DEBUG_ERR( uint(id) >= custom_attrib_loc_to_real_loc.size() );
	DEBUG_ERR( custom_attrib_loc_to_real_loc[id] == -1 );
	return custom_attrib_loc_to_real_loc[id];
}


/*
=======================================================================================================================================
LocTexUnit                                                                                                                            =
=======================================================================================================================================
*/
void shader_prog_t::LocTexUnit( int loc, const texture_t& tex, uint tex_unit ) const
{
	DEBUG_ERR( loc == -1 );
	DEBUG_ERR( GetCurrentProgram() != gl_id );
	tex.Bind( tex_unit );
	glUniform1i( loc, tex_unit );
}

void shader_prog_t::LocTexUnit( const char* loc, const texture_t& tex, uint tex_unit ) const
{
	DEBUG_ERR( GetCurrentProgram() != gl_id );
	tex.Bind( tex_unit );
	glUniform1i( GetUniformLocation(loc), tex_unit );
}




