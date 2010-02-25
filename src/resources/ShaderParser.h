#ifndef _SHADERPARSER_H_
#define _SHADERPARSER_H_

#include <limits>
#include "Common.h"


/**
 * The class fills some of the GLSL spec deficiencies. It adds the include preprocessor directive and the support to have all the
 * shaders in the same file. The file that includes all the shaders is called ShaderParser-compatible
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
			int    definedInLine;
			Pragma(): definedInLine(-1) {}
			Pragma( const string& definedInFile_, int definedInLine_ ):
				definedInFile(definedInFile_), definedInLine(definedInLine_)
			{}
		};
		
		struct IncludePragma: Pragma
		{
			string filename;
		};
		
		struct ShaderVarPragma: Pragma
		{
			string name;
			uint   customLoc;
			ShaderVarPragma( const string& definedInFile_, int definedInLine_, const string& name_, uint customLoc_ ):
				Pragma( definedInFile_, definedInLine_ ), name(name_), customLoc(customLoc_)
			{}
		};
	
		struct CodeBeginningPragma: Pragma
		{
			int globalLine; ///< The line number in the ShaderParser-compatible file

			CodeBeginningPragma(): globalLine(-1) {}
		};
		
		Vec<string> sourceLines;  ///< The parseFileForPragmas fills this
		CodeBeginningPragma vertShaderBegins;
		CodeBeginningPragma fragShaderBegins;
		
		/**
		 * A recursive function that parses a file for pragmas and updates the output
		 * @param filename The file to parse
		 * @param id The #line in GLSL does not support filename so an id it being used instead
		 * @return True on success
		 */
		bool parseFileForPragmas( const string& filename, int id = 0 );

		/**
		 * Searches inside the Output::uniforms or Output::attributes vectors
		 * @param vec Output::uniforms or Output::attributes
		 * @param name The name of the location
		 * @return Iterator to the vector
		 */
		Vec<ShaderVarPragma>::iterator findShaderVar( Vec<ShaderVarPragma>& vec, const string& name ) const;

		void printSourceLines() const;  ///< For debugging
		void printShaderVars() const;  ///< For debugging

		/**
		 * The output of the class packed in this struct
		 */
		struct Output
		{
			friend class ShaderParser;

			private:
				Vec<ShaderVarPragma> uniforms;  ///< It holds the name and the custom location
				Vec<ShaderVarPragma> attributes;  ///< It holds the name and the custom location
				string vertShaderSource; ///< This is the vert shader source
				string fragShaderSource; ///< This is the frag shader source

			public:
				const Vec<ShaderVarPragma>& getUniLocs() const { return uniforms; }
				const Vec<ShaderVarPragma>& getAttribLocs() const { return attributes; }
				const string& getVertShaderSource() const { return vertShaderSource; }
				const string& getFragShaderSource() const { return fragShaderSource; }
		};

		Output output; ///< The output of the parser. parseFile fills it

	public:
		ShaderParser() {}
		~ShaderParser() {}

		/**
		 * Parse a ShaderParser formated GLSL file. Use getOutput to get the output
		 * @param fname The file to parse
		 * @return True on success
		 */
		bool parseFile( const char* fname );

		/**
		 * Wrapper func to get the output. Use it after calling parseFile
		 * @return The output
		 */
		const Output& getOutput() const { return output; }
};


#endif
