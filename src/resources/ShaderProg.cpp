#include "ShaderProg.h"
#include "renderer.h"
#include "ShaderParser.h"
#include "Texture.h"


#define SHADER_ERROR( x ) ERROR( "Shader (" << getRsrcName() << "): " << x )
#define SHADER_WARNING( x ) WARNING( "Shader (" << getRsrcName() << "): " << x )



//=====================================================================================================================================
// createAndCompileShader                                                                                                             =
//=====================================================================================================================================
uint ShaderProg::createAndCompileShader( const char* source_code, const char* preproc, int type ) const
{
	uint glId = 0;
	const char* source_strs[2] = {NULL, NULL};

	// create the shader
	glId = glCreateShader( type );

	// attach the source
	source_strs[1] = source_code;
	source_strs[0] = preproc;

	// compile
	glShaderSource( glId, 2, source_strs, NULL );
	glCompileShader( glId );

	int success;
	glGetShaderiv( glId, GL_COMPILE_STATUS, &success );

	if( !success )
	{
		// print info log
		int info_len = 0;
		int chars_written = 0;
		char* info_log = NULL;

		glGetShaderiv( glId, GL_INFO_LOG_LENGTH, &info_len );
		info_log = (char*)malloc( (info_len+1)*sizeof(char) );
		glGetShaderInfoLog( glId, info_len, &chars_written, info_log );
		
		const char* shader_type;
		switch( type )
		{
			case GL_VERTEX_SHADER:
				shader_type = "Vertex shader";
				break;
			case GL_FRAGMENT_SHADER:
				shader_type = "Fragment shader";
				break;
			default:
				DEBUG_ERR( 1 ); // Not supported
		}
		SHADER_ERROR( shader_type << " compiler log follows:\n" << info_log );
		
		free( info_log );
		return 0;
	}

	return glId;
}


//=====================================================================================================================================
// link                                                                                                                               =
//=====================================================================================================================================
bool ShaderProg::link()
{
	// link
	glLinkProgram( glId );

	// check if linked correctly
	int success;
	glGetProgramiv( glId, GL_LINK_STATUS, &success );

	if( !success )
	{
		int info_len = 0;
		int chars_written = 0;
		char* info_log_txt = NULL;

		glGetProgramiv( glId, GL_INFO_LOG_LENGTH, &info_len );

		info_log_txt = (char*)malloc( (info_len+1)*sizeof(char) );
		glGetProgramInfoLog( glId, info_len, &chars_written, info_log_txt );
		SHADER_ERROR( "Link log follows:\n" << info_log_txt );
		free( info_log_txt );
		return false;
	}

	return true;
}


//=====================================================================================================================================
// getUniAndAttribLocs                                                                                                                =
//=====================================================================================================================================
void ShaderProg::getUniAndAttribLocs()
{
	int num;
	char name_[256];
	GLsizei length;
	GLint size;
	GLenum type;


	// attrib locations
	glGetProgramiv( glId, GL_ACTIVE_ATTRIBUTES, &num );
	for( int i=0; i<num; i++ ) // loop all attributes
	{
		glGetActiveAttrib( glId, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_ );
		name_[ length ] = '\0';

		// check if its FFP location
		int loc = glGetAttribLocation(glId, name_);
		if( loc == -1 )
		{
			//SHADER_WARNING( "You are using FFP vertex attributes (\"" << name_ << "\")" );
			continue;
		}

		attribNameToLoc[ name_ ] = loc;
	}


	// uni locations
	glGetProgramiv( glId, GL_ACTIVE_UNIFORMS, &num );
	for( int i=0; i<num; i++ ) // loop all uniforms
	{
		glGetActiveUniform( glId, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_ );
		name_[ length ] = '\0';

		// check if its FFP location
		int loc = glGetUniformLocation(glId, name_);
		if( loc == -1 )
		{
			//SHADER_WARNING( "You are using FFP uniforms (\"" << name_ << "\")" );
			continue;
		}

		uniNameToLoc[ name_ ] = loc;
	}
}


//=====================================================================================================================================
// fillTheCustomLocationsVectors                                                                                                      =
//=====================================================================================================================================
bool ShaderProg::fillTheCustomLocationsVectors( const ShaderParser& pars )
{
	bind();
	uint max = 0;

	// uniforms
	for( uint i=0; i<pars.getOutput().getUniLocs().size(); ++i )
	{
		if( pars.getOutput().getUniLocs()[i].customLoc > max )
			max = pars.getOutput().getUniLocs()[i].customLoc;
	}
	customUniLocToRealLoc.assign( max + 1, -1 );

	for( uint i=0; i<pars.getOutput().getUniLocs().size(); ++i )
	{
		if( customUniLocToRealLoc[ pars.getOutput().getUniLocs()[i].customLoc ] != -1 )
		{
			SHADER_ERROR( "The uniform \"" << pars.getOutput().getUniLocs()[i].name << "\" has the same value with another one" );
			return false;
		}
		int loc = getUniLoc( pars.getOutput().getUniLocs()[i].name.c_str() );
		if( loc == -1 )
		{
			SHADER_WARNING( "Check the previous error" );
			continue;
		}
		customUniLocToRealLoc[pars.getOutput().getUniLocs()[i].customLoc] = loc;
	}
	
	
	// attributes
	max = 0;
	
	for( uint i=0; i<pars.getOutput().getAttribLocs().size(); ++i )
	{
		if( pars.getOutput().getAttribLocs()[i].customLoc > max )
			max = pars.getOutput().getAttribLocs()[i].customLoc;
	}
	customAttribLocToRealLoc.assign( max + 1, -1 );

	for( uint i=0; i<pars.getOutput().getAttribLocs().size(); ++i )
	{
		if( customAttribLocToRealLoc[ pars.getOutput().getAttribLocs()[i].customLoc ] != -1 )
		{
			SHADER_ERROR( "The attribute \"" << pars.getOutput().getAttribLocs()[i].name << "\" has the same value with another one" );
			return false;
		}
		int loc = getAttribLoc( pars.getOutput().getAttribLocs()[i].name.c_str() );
		if( loc == -1 )
		{
			SHADER_ERROR( "Check the previous error" );
			return false;
		}
		customAttribLocToRealLoc[pars.getOutput().getAttribLocs()[i].customLoc] = loc;
	}

	return true;
}


//=====================================================================================================================================
// load                                                                                                                               =
//=====================================================================================================================================
bool ShaderProg::load( const char* filename )
{
	if( !customload( filename, "" ) ) return false;
	return true;
}


//=====================================================================================================================================
// customload                                                                                                                         =
//=====================================================================================================================================
bool ShaderProg::customload( const char* filename, const char* extra_source )
{
	if( getRsrcName().length() == 0 )
	{
		name = Util::cutPath( filename );
		path = Util::getPath( filename );
	}

	ShaderParser pars;

	if( !pars.parseFile( filename ) ) return false;

	// create, compile, attach and link
	string preproc_source = r::GetStdShaderPreprocDefines() + extra_source;
	uint vert_glId = createAndCompileShader( pars.getOutput().getVertShaderSource().c_str(), preproc_source.c_str(), GL_VERTEX_SHADER );
	if( vert_glId == 0 ) return false;

	uint frag_glId = createAndCompileShader( pars.getOutput().getFragShaderSource().c_str(), preproc_source.c_str(), GL_FRAGMENT_SHADER );
	if( frag_glId == 0 ) return false;

	glId = glCreateProgram();
	glAttachShader( glId, vert_glId );
	glAttachShader( glId, frag_glId );

	if( !link() ) return false;
	
	
	// init the rest
	getUniAndAttribLocs();
	if( !fillTheCustomLocationsVectors( pars ) ) return false;

	return true;
}


//=====================================================================================================================================
// getUniLoc                                                                                                                 =
//=====================================================================================================================================
int ShaderProg::getUniLoc( const char* name ) const
{
	DEBUG_ERR( glId == 0 ); // not initialized
	NameToLocIterator it = uniNameToLoc.find( name );
	if( it == uniNameToLoc.end() )
	{
		SHADER_ERROR( "Cannot get uniform loc \"" << name << '\"' );
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// getAttribLoc                                                                                                               =
//=====================================================================================================================================
int ShaderProg::getAttribLoc( const char* name ) const
{
	DEBUG_ERR( glId == 0 ); // not initialized
	NameToLocIterator it = attribNameToLoc.find( name );
	if( it == attribNameToLoc.end() )
	{
		SHADER_ERROR( "Cannot get attrib loc \"" << name << '\"' );
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// getUniLocSilently                                                                                                         =
//=====================================================================================================================================
int ShaderProg::getUniLocSilently( const char* name ) const
{
	DEBUG_ERR( glId == 0 ); // not initialized
	NameToLocIterator it = uniNameToLoc.find( name );
	if( it == uniNameToLoc.end() )
	{
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// getAttribLocSilently                                                                                                       =
//=====================================================================================================================================
int ShaderProg::getAttribLocSilently( const char* name ) const
{
	DEBUG_ERR( glId == 0 ); // not initialized
	NameToLocIterator it = attribNameToLoc.find( name );
	if( it == attribNameToLoc.end() )
	{
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// getUniLoc                                                                                                                 =
//=====================================================================================================================================
int ShaderProg::GetUniLoc( int id ) const
{
	DEBUG_ERR( uint(id) >= customUniLocToRealLoc.size() );
	DEBUG_ERR( customUniLocToRealLoc[id] == -1 );
	return customUniLocToRealLoc[id];
}


//=====================================================================================================================================
// getAttribLoc                                                                                                               =
//=====================================================================================================================================
int ShaderProg::getAttribLoc( int id ) const
{
	DEBUG_ERR( uint(id) >= customAttribLocToRealLoc.size() );
	DEBUG_ERR( customAttribLocToRealLoc[id] == -1 );
	return customAttribLocToRealLoc[id];
}


//=====================================================================================================================================
// locTexUnit                                                                                                                         =
//=====================================================================================================================================
void ShaderProg::locTexUnit( int loc, const Texture& tex, uint tex_unit ) const
{
	DEBUG_ERR( loc == -1 );
	DEBUG_ERR( getCurrentProgram() != glId );
	tex.bind( tex_unit );
	glUniform1i( loc, tex_unit );
}

void ShaderProg::locTexUnit( const char* loc, const Texture& tex, uint tex_unit ) const
{
	DEBUG_ERR( getCurrentProgram() != glId );
	tex.bind( tex_unit );
	glUniform1i( getUniLoc(loc), tex_unit );
}
