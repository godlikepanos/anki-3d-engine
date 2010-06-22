#ifndef _SHADERPARSER_H_
#define _SHADERPARSER_H_

#include <limits>
#include "Common.h"


/**
 * Helper class used for shader program loading
 *
 * The class fills some of the GLSL spec deficiencies. It adds the include preprocessor directive and the support to
 * have all the shaders in the same file. The file that includes all the shaders is called
 * ShaderPrePreprocessor-compatible. The preprocessor pragmas are four: include, vertShaderBegins, fragShaderBegins and
 * attribute. The *ShaderBegins indicate where the shader code begins and must be in certain order, first the vert
 * shader then the optional geom and then the frag. The include is self-explanatory. The attribute is used to bind
 * custom locations to attributes.
 */
class ShaderPrePreprocessor
{
	//====================================================================================================================
	// Nested                                                                                                            =
	//====================================================================================================================
	protected:
		/**
		 * The pragma base class
		 */
		struct Pragma
		{
			
			string definedInFile;
			int    definedInLine;
			Pragma();
			Pragma(const string& definedInFile_, int definedInLine_);
		};
		
		struct IncludePragma: Pragma
		{
			string filename;
		};
		
		struct ShaderVarPragma: Pragma
		{
			string name;
			uint   customLoc;

			ShaderVarPragma(const string& definedInFile_, int definedInLine_, const string& name_, uint customLoc_);
		};

		struct TrffbVaryingPragma: Pragma
		{
			string name;
			uint   id;

			TrffbVaryingPragma(const string& definedInFile_, int definedInLine_, const string& name_, uint id_);
		};
	
		struct CodeBeginningPragma: Pragma
		{
			int globalLine; ///< The line number in the ShaderPrePreprocessor-compatible file

			CodeBeginningPragma();
		};

		/**
		 * The output of the class packed in this struct
		 */
		struct Output
		{
			friend class ShaderPrePreprocessor;

			PROPERTY_R(Vec<ShaderVarPragma>, attributes, getAttribLocs) ///< It holds the name and the custom location
			/**
			 * Names and and ids for transform feedback varyings
			 */
			PROPERTY_R(Vec<TrffbVaryingPragma>, trffbVaryings, getTrffbVaryings)
			PROPERTY_R(string, vertShaderSource, getVertShaderSource) ///< The vert shader source
			PROPERTY_R(string, geomShaderSource, getGeomShaderSource) ///< The geom shader source
			PROPERTY_R(string, fragShaderSource, getFragShaderSource) ///< The frag shader source
		};

	//====================================================================================================================
	// Properties                                                                                                        =
	//====================================================================================================================
	PROPERTY_R(Output, output, getOutput) ///< The one and only public property

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		ShaderPrePreprocessor() {}
		~ShaderPrePreprocessor() {}

		/**
		 * Parse a ShaderPrePreprocessor formated GLSL file. Use getOutput to get the output
		 * @param fname The file to parse
		 * @return True on success
		 */
		bool parseFile(const char* fname);

	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	protected:
		Vec<string> sourceLines;  ///< The parseFileForPragmas fills this
		CodeBeginningPragma vertShaderBegins;
		CodeBeginningPragma geomShaderBegins;
		CodeBeginningPragma fragShaderBegins;
		
		/**
		 * A recursive function that parses a file for pragmas and updates the output
		 * @param filename The file to parse
		 * @param depth The #line in GLSL does not support filename so an depth it being used. It also tracks the includance
		 * depth
		 * @return True on success
		 */
		bool parseFileForPragmas(const string& filename, int depth = 0);

		/**
		 * Searches inside the Output::uniforms or Output::attributes vectors
		 * @param vec Output::uniforms or Output::attributes
		 * @param name The name of the location
		 * @return Iterator to the vector
		 */
		Vec<ShaderVarPragma>::iterator findShaderVar(Vec<ShaderVarPragma>& vec, const string& name) const;

		/**
		 * Searches inside the Output::attributes or  vectors
		 * @param vec Output::uniforms or Output::attributes
		 * @param name The name of the location
		 * @return Iterator to the vector
		 */
		template<typename Type> typename Vec<Type>::iterator findNamed(const Vec<Type>& vec, const string& what) const;

		void printSourceLines() const;  ///< For debugging
		void printShaderVars() const;  ///< For debugging
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================
inline ShaderPrePreprocessor::Pragma::Pragma():
	definedInLine(-1)
{}


inline ShaderPrePreprocessor::Pragma::Pragma(const string& definedInFile_, int definedInLine_):
	definedInFile(definedInFile_),
	definedInLine(definedInLine_)
{}


inline ShaderPrePreprocessor::ShaderVarPragma::ShaderVarPragma(const string& definedInFile_, int definedInLine_,
                                                               const string& name_, uint customLoc_):
	Pragma(definedInFile_, definedInLine_),
	name(name_),
	customLoc(customLoc_)
{}


inline ShaderPrePreprocessor::TrffbVaryingPragma::TrffbVaryingPragma(const string& definedInFile_,
                                                                         int definedInLine_,
                                                                         const string& name_, uint id_):
	Pragma(definedInFile_, definedInLine_),
	name(name_),
	id(id_)
{}


inline ShaderPrePreprocessor::CodeBeginningPragma::CodeBeginningPragma():
	globalLine(-1)
{}


template<typename Type> typename Vec<Type>::iterator ShaderPrePreprocessor::findNamed(const Vec<Type>& vec,
                                                                                      const string& what) const
{
	typename Vec<Type>::iterator it = vec.begin();
	while(it != vec.end() && it->name != what)
	{
		++it;
	}
	return it;
}


#endif
