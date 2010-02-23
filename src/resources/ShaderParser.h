#ifndef _SHADERPARSER_H_
#define _SHADERPARSER_H_

#include <limits>
#include "common.h"


/**
 * The class fills some of the GLSL spec deficiencies. It adds the include preprocessor directive and the support to have all the
 * shaders in the same file
 */
class ShaderParser
{
	protected:
		/**
		 * The pragma base class
		 */
		struct Pragma
		{
			
			string definedInFile;
			int    defined_in_line;
			Pragma(): defined_in_line(-1) {}
			Pragma( const string& definedInFile_, int defined_in_line_ ):
				definedInFile(definedInFile_), defined_in_line(defined_in_line_)
			{}
		};
		
		struct IncludePragma: Pragma
		{
			string filename;
		};
		
		struct ShaderVarPragma: Pragma
		{
			string name;
			uint   custom_loc;
			ShaderVarPragma( const string& defined_in_file_, int defined_in_line_, const string& name_, uint custom_loc_ ):
				Pragma( defined_in_file_, defined_in_line_ ), name(name_), custom_loc(custom_loc_)
			{}
		};
	
		struct code_beginning_t: Pragma
		{
			int global_line;

			code_beginning_t(): global_line(-1) {}
		};
		
		vec_t<string> source_lines;
		code_beginning_t vert_shader_begins;
		code_beginning_t frag_shader_begins;
		
		bool ParseFileForPragmas( const string& filename, int id = 0 );
		vec_t<ShaderVarPragma>::iterator FindShaderVar( vec_t<ShaderVarPragma>& vec, const string& name ) const;
		void PrintSourceLines() const;  ///< For debugging
		void PrintShaderVars() const;  ///< For debugging

	public:
		// the output
		vec_t<ShaderVarPragma> uniforms;
		vec_t<ShaderVarPragma> attributes;
		string vert_shader_source;
		string frag_shader_source;
		
		ShaderParser() {}
		~ShaderParser() {}

		bool ParseFile( const char* fname ); ///< Parse a file and its includes 
};


#endif
