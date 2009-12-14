#ifndef _SHADER_PARSER_H_
#define _SHADER_PARSER_H_

#include "common.h"


class shader_parser_t
{
	PROPERTY_R( uint, gl_id, GetGLID )

	protected:
		typedef pair<string, int> str_int_pair_t;

		struct included_file_t
		{
			string fname;
			int line;
			int shader_type; ///< GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
			included_file_t( const string& fname_, int line_ ): fname(fname_), line(line_) {}
		};
		
		struct location_t
		{
			string name;
			uint custom_loc;
			location_t( const string& name_, int custom_loc_ ): name(name_), custom_loc(custom_loc_) {}
		};
	
		bool ParseFileForPragmas( const char* filename, int& start_line_vert, int& start_line_frag,
		                          vec_t<included_file_t>& includes, vec_t<location_t>& uniforms, vec_t<location_t>& attributes ) const;
		bool ParseFileForStartPragmas( const char* filename, int& start_line_vert, int& start_line_frag ) const;

	public:
		shader_parser_t(): gl_id(0) {}

		bool Load( const char* fname );
		void Unload() {}
};


#endif
