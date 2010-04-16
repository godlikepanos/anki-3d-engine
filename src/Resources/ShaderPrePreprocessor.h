#ifndef _SHADERPARSER_H_
#define _SHADERPARSER_H_

#include <limits>
#include "Common.h"


/**
 * @brief Helper class used for shader program loading
 *
 * The class fills some of the GLSL spec deficiencies. It adds the include preprocessor directive and the support to have all the
 * shaders in the same file. The file that includes all the shaders is called ShaderPrePreprocessor-compatible. The preprocessor pragmas are
 * four: include, vertShaderBegins, fragShaderBegins and attribute. The *ShaderBegins indicate where the shader code begins and must be
 * in certain order, first the vert shader and then the frag. The include is self-explanatory. The attribute is used to bind custom
 * locations to attributes.
 */
class ShaderPrePreprocessor
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
			int globalLine; ///< The line number in the ShaderPrePreprocessor-compatible file

			CodeBeginningPragma(): globalLine(-1) {}
		};
		
		Vec<string> sourceLines;  ///< The parseFileForPragmas fills this
		CodeBeginningPragma vertShaderBegins;
		CodeBeginningPragma geomShaderBegins;
		CodeBeginningPragma fragShaderBegins;
		
		/**
		 * A recursive function that parses a file for pragmas and updates the output
		 * @param filename The file to parse
		 * @param depth The #line in GLSL does not support filename so an depth it being used. It also tracks the include depth
		 * @return True on success
		 */
		bool parseFileForPragmas( const string& filename, int depth = 0 );

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
			friend class ShaderPrePreprocessor;

			PROPERTY_R( Vec<ShaderVarPragma>, attributes, getAttribLocs ) ///< It holds the name and the custom location
			PROPERTY_R( string, vertShaderSource, getVertShaderSource ) ///< The vert shader source
			PROPERTY_R( string, geomShaderSource, getGeomShaderSource ) ///< The geom shader source
			PROPERTY_R( string, fragShaderSource, getFragShaderSource ) ///< The frag shader source
		};

		Output output; ///< The output of the parser. parseFile fills it

	public:
		ShaderPrePreprocessor() {}
		~ShaderPrePreprocessor() {}

		/**
		 * Parse a ShaderPrePreprocessor formated GLSL file. Use getOutput to get the output
		 * @param fname The file to parse
		 * @return True on success
		 */
		bool parseFile( const char* fname );

		/**
		 * Accessor func to get the output. Use it after calling parseFile
		 * @return The output
		 */
		const Output& getOutput() const { return output; }
};


#endif
