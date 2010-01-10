#ifndef _SHADER_PARSER_H_
#define _SHADER_PARSER_H_

#include <limits>
#include "common.h"


class shader_parser_t
{
	protected:		
		struct pragma_t
		{
			
			string defined_in_file;
			int    defined_in_line;
			pragma_t(): defined_in_line(-1) {}
			pragma_t( const string& defined_in_file_, int defined_in_line_ ): 
				defined_in_file(defined_in_file_), defined_in_line(defined_in_line_)
			{}
		};
		
		struct include_t: pragma_t
		{
			string filename;
		};
		
		struct shader_var_t: pragma_t
		{
			string name;
			uint   custom_loc;
			shader_var_t( const string& defined_in_file_, int defined_in_line_, const string& name_, uint custom_loc_ ): 
				pragma_t( defined_in_file_, defined_in_line_ ), name(name_), custom_loc(custom_loc_)
			{}
		};
	
		struct code_beginning_t: pragma_t
		{
			int global_line;

			code_beginning_t(): global_line(-1) {}
		};
		
		vec_t<string> source_lines;
		code_beginning_t vert_shader_begins;
		code_beginning_t frag_shader_begins;
		
		bool ParseFileForPragmas( const string& filename, int id = 0 );
		vec_t<shader_var_t>::iterator FindShaderVar( vec_t<shader_var_t>& vec, const string& name ) const;
		void PrintSourceLines() const;  ///< For debugging
		void PrintShaderVars() const;  ///< For debugging

	public:
		// the output
		vec_t<shader_var_t> uniforms;
		vec_t<shader_var_t> attributes;
		string vert_shader_source;
		string frag_shader_source;
		
		shader_parser_t() {}
		~shader_parser_t() {}

		bool ParseFile( const char* fname ); ///< Parse a file and its includes 
};


#endif
