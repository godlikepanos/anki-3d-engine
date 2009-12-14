#include "shaders.h"
#include "scanner.h"
#include "renderer.h"


/*
=======================================================================================================================================
LoadAndCompile                                                                                                                        =
=======================================================================================================================================
*/
bool shader_t::LoadAndCompile( const char* filename, int type_, const char* preprocessor_str )
{
#ifdef _USE_SHADERS_
	type = type_;
	const char* source_strs[2] = {NULL, NULL};

	source_strs[0] = preprocessor_str;

	// create the shader
	gl_id = glCreateShader( type );

	// attach the source
	source_strs[1] = ReadFile( filename );
	if( source_strs[1] == NULL )
	{
		ERROR( "Cannot read file \"" << filename << '\"' );
		return false;
	}

	glShaderSource( gl_id, 2, source_strs, NULL );
	delete [] source_strs[1];

	// compile
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
		ERROR( "Shader \"" << filename << "\" compile log follows:\n" << info_log );
		free( info_log );
		return false;
	}
#endif
	return true;
}


/*
=======================================================================================================================================
AttachShader                                                                                                                          =
=======================================================================================================================================
*/
void shader_prog_t::AttachShader( const shader_t& shader )
{
#ifdef _USE_SHADERS_
	if( gl_id == 0 )
		gl_id = glCreateProgram();

	glAttachShader( gl_id, shader.gl_id );
#endif
}


/*
=======================================================================================================================================
Link                                                                                                                                  =
=======================================================================================================================================
*/
bool shader_prog_t::Link()
{
#ifdef _USE_SHADERS_
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
		ERROR( "Shader program " << GetName() << " link log follows:\n" << info_log_txt );
		free( info_log_txt );
		return false;
	}
#endif

	return true;
}


/*
=======================================================================================================================================
LoadCompileLink                                                                                                                       =
=======================================================================================================================================
*/
bool shader_prog_t::LoadCompileLink( const char* vert_fname, const char* frag_fname, const char* preprocessor_str )
{
#ifdef _USE_SHADERS_
	shader_t shaders[2];

	if( !shaders[0].LoadAndCompile( vert_fname, GL_VERTEX_SHADER, preprocessor_str ) ) return false;

	if( !shaders[1].LoadAndCompile( frag_fname, GL_FRAGMENT_SHADER, preprocessor_str ) ) return false;

	AttachShader( shaders[0] );
	AttachShader( shaders[1] );

	return Link();
#endif

	return true;
}

/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool shader_prog_t::Load( const char* filename )
{
	if( !scn::LoadFile( filename ) ) return false;

	string vert = "", frag = "", pproc = "";
	vector<string> uniforms;
	const scn::token_t* token;


	do
	{
		token = &scn::GetNextToken();

		//** vert shader **
		if( token->code == scn::TC_IDENTIFIER && !strcmp( token->value.string, "VERTEX_SHADER" ) )
		{
			token = &scn::GetNextToken();
			if( token->code != scn::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			vert = token->value.string;
		}
		//** frag shader **
		else if( token->code == scn::TC_IDENTIFIER && !strcmp( token->value.string, "FRAGMENT_SHADER" ) )
		{
			token = &scn::GetNextToken();
			if( token->code != scn::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			frag = token->value.string;
		}
		//** preprocessor defines **
		else if( token->code == scn::TC_IDENTIFIER && !strcmp( token->value.string, "PREPROCESSOR_DEFINES" ) )
		{
			token = &scn::GetNextToken();
			// {
			if( token->code != scn::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			// the defines as strings
			do
			{
				token = &scn::GetNextToken();

				if( token->code == scn::TC_RBRACKET )
					break;
				else if( token->code == scn::TC_STRING )
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
		//** uniform vars **
		else if( token->code == scn::TC_IDENTIFIER && !strcmp( token->value.string, "UNIFORM_VARS" ) )
		{
			token = &scn::GetNextToken();
			// {
			if( token->code != scn::TC_LBRACKET )
			{
				PARSE_ERR_EXPECTED( "{" );
				return false;
			}

			// for all the uniform var's name as string
			do
			{
				token = &scn::GetNextToken();

				if( token->code == scn::TC_RBRACKET )
					break;
				else if( token->code == scn::TC_STRING )
					uniforms.push_back( token->value.string );
				else
				{
					PARSE_ERR_EXPECTED( "string" );
					return false;
				}

			}while( true );
		}
		// end of file
		else if( token->code == scn::TC_EOF )
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

	scn::UnloadFile();

	// add the standard preprocessor defines from renderer. Note that these defines overpower the shader ones in case of a re-definition
	pproc += r::GetStdShaderPreprocDefines();

	// ready the shader for use
	if( !LoadCompileLink( vert.c_str(), frag.c_str(), pproc.c_str() ) ) return false;

#ifdef _USE_SHADERS_

	// bind the uniform vars
	Bind();
	for( uint i=0; i<uniforms.size(); i++ )
	{
		int loc = GetUniLocation( uniforms[i].c_str() );
		/*if( loc == -1 )
		{
			ERROR( "Cannot get uniform loc \"" << uniforms[i] << '\"' );
			return false;
		}*/
		uniform_locations.push_back( loc );
	}
	UnBind();

#endif

	return true;
}


/*
=======================================================================================================================================
GetUniLocation                                                                                                                        =
=======================================================================================================================================
*/
int shader_prog_t::GetUniLocation( const char* name )
{
#ifdef _USE_SHADERS_
	GLint loc = glGetUniformLocation( gl_id, name );
	if( loc == -1 ) ERROR( "Cannot get uniform loc \"" << name << '\"' );
	return loc;
#endif
}


/*
=======================================================================================================================================
GetUniLocation                                                                                                                        =
=======================================================================================================================================
*/
int shader_prog_t::GetAttrLocation( const char* name )
{
#ifdef _USE_SHADERS_
	GLint loc = glGetAttribLocation( gl_id, name );
	if( loc == -1 ) ERROR( "Cannot get attrib location \"" << name << '\"' );
	return loc;
#endif
}





