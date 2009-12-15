#include "shader_prog.h"
#include "shader_parser.h"


#define SHADER_ERROR( x ) ERROR( "Shader prog \"" << GetName() << "\": " << x )
#define SHADER_WARNING( x ) WARNING( "Shader prog \"" << GetName() << "\": " << x )


//=====================================================================================================================================
// CreateAndCompileShader                                                                                                             =
//=====================================================================================================================================
uint shader_prog_t::CreateAndCompileShader( const char* source_code, int type ) const
{
	uint gl_id = 0;
	const char* source_strs[2] = {NULL, NULL};

	// create the shader
	gl_id = glCreateShader( type );

	// attach the source
	source_strs[0] = source_code;
	source_strs[1] = r::GetStdShaderPreprocDefines().c_str();

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
		
		char* shader_type;
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

	return gl_id;
}


//=====================================================================================================================================
// Link                                                                                                                               =
//=====================================================================================================================================
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


//=====================================================================================================================================
// GetUniAndAttribLocs                                                                                                                =
//=====================================================================================================================================
void shader_prog_t::GetUniAndAttribLocs()
{
	int num;
	char name_[256];
	GLsizei length;
	GLint size;
	GLenum type;

	// attrib locations
	glGetProgramiv( gl_id, GL_ACTIVE_ATTRIBUTES, &num );
	for( int i=0; i<num; i++ ) // loop all attributes
	{
		glGetActiveAttrib( gl_id, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_ );
		name_[ length ] = '\0';

		// check if its FFP location
		if( GetAttributeLocationSilently(name_) == -1 )
		{
			//SHADER_WARNING( "You are using FFP vertex attributes (\"" << name_ << "\")" );
			continue;
		}

		attrib_name_to_loc[ name_ ] = i
	}


	// uni locations
	glGetProgramiv( gl_id, GL_ACTIVE_UNIFORMS, &num );
	for( int i=0; i<num; i++ ) // loop all uniforms
	{
		glGetActiveUniform( gl_id, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_ );
		name_[ length ] = '\0';

		// check if its FFP location
		if( GetUniformLocationSilently(name_) == -1 )
		{
			//SHADER_WARNING( "You are using FFP vertex attributes (\"" << name_ << "\")" );
			continue;
		}

		uni_name_to_loc[ name_ ] = i
	}
}


//=====================================================================================================================================
// FillTheCustomLocationsVectors                                                                                                      =
//=====================================================================================================================================
bool shader_prog_t::FillTheCustomLocationsVectors( const shader_parser_t& pars )
{
	int max = 0;

	// uniforms
	for( uint i=0; i<pars.uniforms.size(); ++i )
	{
		if( pars.uniforms[i].custom_loc > max ) 
			max = pars.uniforms[i].custom_loc;
	}
	custom_uni_loc_to_real_loc.assign( max + 1, -1 );

	for( uint i=0; i<pars.uniforms.size(); ++i )
	{
		if( custom_uni_loc_to_real_loc[ pars.uniforms[i].custom_loc ] != -1 )
		{
			SHADER_ERROR( "The uniform \"" << pars.uniforms[i].name << "\" has the same value with another one" );
			return false;
		}
		int loc = GetUniformLocation( pars.uniforms[i].name.c_str() );
		if( loc == -1 )
		{
			SHADER_ERROR( "Check the previous error" );
			return false;
		}
		custom_uni_loc_to_real_loc[pars.uniforms[i].custom_loc] = loc;
	}
	
	
	// attributes
	max = 0;
	
	for( uint i=0; i<pars.attributes.size(); ++i )
	{
		if( pars.attributes[i].custom_loc > max ) 
			max = pars.attributes[i].custom_loc;
	}
	custom_attr_loc_to_real_loc.assign( max + 1, -1 );

	for( uint i=0; i<pars.attributes.size(); ++i )
	{
		if( custom_attr_loc_to_real_loc[ pars.attributes[i].custom_loc ] != -1 )
		{
			SHADER_ERROR( "The attribute \"" << pars.attributes[i].name << "\" has the same value with another one" );
			return false;
		}
		int loc = GetAttributeLocation( pars.attributes[i].name.c_str() );
		if( loc == -1 )
		{
			SHADER_ERROR( "Check the previous error" );
			return false;
		}
		custom_attr_loc_to_real_loc[pars.attributes[i].custom_loc] = loc;
	}

	return true;
}


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool shader_prog_t::Load( const char* filename )
{
	shader_parser_t pars;

	if( !pars.LoadFile( filename ) ) return false;


	// create, compile, attach and link
	uint vert_gl_id = CreateAndCompileShader( pars.vert_shader_source.c_str(), GL_VERTEX_SHADER );
	if( vert_gl_id == 0 ) return false;

	uint frag_gl_id = CreateAndCompileShader( pars.frag_shader_source.c_str(), GL_FRAGMENT_SHADER );
	if( frag_gl_id == 0 ) return false;

	gl_id = glCreateProgram();
	glAttachShader( gl_id, vert_gl_id );
	glAttachShader( gl_id, frag_gl_id );

	if( !Link() ) return false;
	
	
	// init the rest
	GetUniAndAttribLocs();
	if( !FillTheCustomLocationsVectors( pars ) ) return false;
	
	return true;
}


//=====================================================================================================================================
// GetUniformLocation                                                                                                                 =
//=====================================================================================================================================
int shader_prog_t::GetUniformLocation( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 ); // not initialized
	ntlit_t it = uni_name_to_loc.find( name );
	if( it == uni_name_to_loc.end() )
	{
		SHADER_ERROR( "Cannot get uniform loc \"" << name << '\"' );
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// GetAttributeLocation                                                                                                               =
//=====================================================================================================================================
int shader_prog_t::GetAttributeLocation( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 ); // not initialized
	ntlit_t it = attrib_name_to_loc.find( name );
	if( it == attrib_name_to_loc.end() )
	{
		SHADER_ERROR( "Cannot get attrib loc \"" << name << '\"' );
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// GetUniformLocationSilently                                                                                                         =
//=====================================================================================================================================
int shader_prog_t::GetUniformLocationSilently( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 ); // not initialized
	ntlit_t it = uni_name_to_loc.find( name );
	if( it == uni_name_to_loc.end() )
	{
		return -1;
	}
	return it->second;
}


//=====================================================================================================================================
// GetAttributeLocationSilently                                                                                                       =
//=====================================================================================================================================
int shader_prog_t::GetAttributeLocationSilently( const char* name ) const
{
	DEBUG_ERR( gl_id == 0 ); // not initialized
	ntlit_t it = attrib_name_to_loc.find( name );
	if( it == attrib_name_to_loc.end() )
	{
		return -1;
	}
	return it->second;
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


//=====================================================================================================================================
// LocTexUnit                                                                                                                         =
//=====================================================================================================================================
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
