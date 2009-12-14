#include "shader_prog.h"
#include "scanner.h"
#include "parser.h"


#define SHADER_ERROR( x ) ERROR( "Shader prog \"" << GetName() << "\": " << x )
#define SHADER_WARNING( x ) WARNING( "Shader prog \"" << GetName() << "\": " << x )


//=====================================================================================================================================
// ParseFileForStartPragmas                                                                                                           =
//=====================================================================================================================================
bool shader_parser_t::ParseFileForStartPragmas( const char* filename, int& start_line_vert, int& start_line_frag ) const
{
	// scanner
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;
	const scanner_t::token_t* token;

	// locals
	start_line_vert = -1;
	start_line_frag = -1;

	do
	{
		token = &scanner.GetNextToken();

		if( token->code == scanner_t::TC_SHARP )
		{
			token = &scanner.GetNextToken();
			if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "pragma") == 0 )
			{
				token = &scanner.GetNextToken();
				if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "anki") == 0 )
				{
					token = &scanner.GetNextToken();
					if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "vert_shader_start") == 0 )
					{
						start_line_vert = scanner.GetLineNmbr() - 1;
					}
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "frag_shader_start") == 0 )
					{
						start_line_frag = scanner.GetLineNmbr() - 1;
					}
				} // end if anki
			} // end if pragma
		} // end if #

	} while( token->code != scanner_t::TC_EOF );


	// sanity checks
	if( start_line_vert == -1 )
	{
		SHADER_ERROR( "Cannot find: #pragma anki vert_shader_start" );
		return false;
	}
	if( start_line_frag == -1 )
	{
		SHADER_ERROR( "Cannot find: #pragma anki frag_shader_start" );
		return false;
	}
	if( start_line_vert >= start_line_frag )
	{
		SHADER_ERROR( "Vertex shader code must preside fragment shader code" );
		return false;
	}

	return true;
}


//=====================================================================================================================================
// ParseFileForPragmas                                                                                                                =
//=====================================================================================================================================
bool shader_prog2_t::ParseFileForPragmas( const char* filename, int& start_line_vert, int& start_line_frag, 
                                          vec_t<included_file_t>& includes, vec_t<location_t>& uniforms, vec_t<location_t>& attributes
                                        ) const
{
	// scanner
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;
	const scanner_t::token_t* token;

	// locals
	start_line_vert = -1;
	start_line_frag = -1;
	includes.clear();
	uniforms.clear();
	attributes.clear();

	do
	{
		token = &scanner.GetNextToken();

		if( token->code == scanner_t::TC_SHARP )
		{
			token = &scanner.GetNextToken();
			if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "pragma") == 0 )
			{
				token = &scanner.GetNextToken();
				if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "anki") == 0 )
				{
					token = &scanner.GetNextToken();
					if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "vert_shader_start") == 0 )
					{
						start_line_vert = scanner.GetLineNmbr() - 1;
					}
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "frag_shader_start") == 0 )
					{
						start_line_frag = scanner.GetLineNmbr() - 1;
					}
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "include") == 0 )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_STRING )
						{
							//includes.push_back( make_pair( token->value.string, scanner.GetLineNmbr()-1 ) );
							includes.push_back( included_file_t( token->value.string, scanner.GetLineNmbr()-1 ) );
						}
						else
						{
							PARSE_ERR_EXPECTED( "string" );
							return false;
						}
					}
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "uniform") == 0 )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_IDENTIFIER )
						{
							string uniform = token->value.string;
							token = &scanner.GetNextToken();
							if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_INT )
							{
								uniforms.push_back( location_t( uniform, token->value.int_ ) );
							}
							else
							{
								PARSE_ERR_EXPECTED( "integer" );
								return false;
							}
						}
						else
						{
							PARSE_ERR_EXPECTED( "identifier" );
							return false;
						}
					}
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "attribute") == 0 )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_IDENTIFIER )
						{
							string attribute = token->value.string;
							token = &scanner.GetNextToken();
							if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_INT )
							{
								attributes.push_back( location_t( attribute, token->value.int_ ) );
							}
							else
							{
								PARSE_ERR_EXPECTED( "integer" );
								return false;
							}
						}
						else
						{
							PARSE_ERR_EXPECTED( "identifier" );
							return false;
						}
					}
					else
					{
						PARSE_WARN( "I will ignore this #pragma anki" );
					}
				} // end if anki
			} // end if pragma
		} // end if #

	} while( token->code != scanner_t::TC_EOF );
	
	return true;
}


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool shader_prog2_t::Load( const char* filename )
{
	// locals
	int start_line_vert;
	int start_line_frag;
	vec_t<included_file_t> includes;
	vec_t<location_t> uniforms;
	vec_t<location_t> attributes;

	// parse master file
	ParseFileForStartPragmas( filename, start_line_vert, start_line_frag );

	vec_t<string> lines = GetFileLines( filename );
	for( int i=start_line_vert+1; i<start_line_frag; ++i )
	{
		PRINT( lines[i] );
	}

	return true;
}

