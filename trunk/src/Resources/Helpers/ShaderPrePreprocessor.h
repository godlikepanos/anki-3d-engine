#ifndef SHADER_PARSER_H
#define SHADER_PARSER_H

#include <limits>
#include "Vec.h"
#include "StdTypes.h"
#include "Properties.h"


/// Helper class used for shader program loading
///
/// The class fills some of the GLSL spec deficiencies. It adds the include preprocessor directive and the support to
/// have all the shaders in the same file. The file that includes all the shaders is called
/// ShaderPrePreprocessor-compatible.
///
/// The preprocessor pragmas are:
///
/// - #pragma anki vertShaderBegins
/// - #pragma anki geomShaderBegins
/// - #pragma anki fragShaderBegins
/// - #pragma anki attribute <varName> <customLocation>
/// - #pragma anki include "<filename>"
/// - #pragma anki transformFeedbackVarying <varName>
///
/// @note The order of the *ShaderBegins is important
class ShaderPrePreprocessor
{
	//====================================================================================================================
	// Nested                                                                                                            =
	//====================================================================================================================
	protected:
		/// The pragma base class
		struct Pragma
		{
			std::string definedInFile;
			int    definedInLine;
			Pragma(): definedInLine(-1) {}
			Pragma(const std::string& definedInFile_, int definedInLine_);
		};
		
		struct IncludePragma: Pragma
		{
			std::string filename;
		};
		
		struct ShaderVarPragma: Pragma
		{
			std::string name;
			uint customLoc;

			ShaderVarPragma(const std::string& definedInFile_, int definedInLine_, const std::string& name_, uint customLoc_);
		};

		struct TrffbVaryingPragma: Pragma
		{
			std::string name;

			TrffbVaryingPragma(const std::string& definedInFile_, int definedInLine_, const std::string& name_);
		};
	
		struct CodeBeginningPragma: Pragma
		{
			int globalLine; ///< The line number in the ShaderPrePreprocessor-compatible file

			CodeBeginningPragma(): globalLine(-1) {}
		};

		/// The output of the class packed in this struct
		struct Output
		{
			friend class ShaderPrePreprocessor;

			PROPERTY_R(Vec<ShaderVarPragma>, attributes, getAttribLocs) ///< It holds the name and the custom location
			/// Names and and ids for transform feedback varyings
			PROPERTY_R(Vec<TrffbVaryingPragma>, trffbVaryings, getTrffbVaryings)
			PROPERTY_R(std::string, vertShaderSource, getVertShaderSource) ///< The vert shader source
			PROPERTY_R(std::string, geomShaderSource, getGeomShaderSource) ///< The geom shader source
			PROPERTY_R(std::string, fragShaderSource, getFragShaderSource) ///< The frag shader source
		};

	//====================================================================================================================
	// Properties                                                                                                        =
	//====================================================================================================================
	PROPERTY_R(Output, output, getOutput) ///< The one and only public property

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/// It loads a file and parses it
		/// @param[in] filename The file to load
		/// @exception Exception
		ShaderPrePreprocessor(const char* filename) {parseFile(filename);}

		/// Destructor does nothing
		~ShaderPrePreprocessor() {}

	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	protected:
		Vec<std::string> sourceLines;  ///< The parseFileForPragmas fills this
		CodeBeginningPragma vertShaderBegins;
		CodeBeginningPragma geomShaderBegins;
		CodeBeginningPragma fragShaderBegins;

		/// Parse a ShaderPrePreprocessor formated GLSL file. Use getOutput to get the output
		/// @param filename The file to parse
		/// @exception Ecxeption
		void parseFile(const char* filename);

		/// A recursive function that parses a file for pragmas and updates the output
		/// @param filename The file to parse
		/// @param depth The #line in GLSL does not support filename so an depth it being used. It also tracks the includance
		/// depth
		/// @exception Ecxeption
		void parseFileForPragmas(const std::string& filename, int depth = 0);

		/// Searches inside the Output::uniforms or Output::attributes vectors
		/// @param vec Output::uniforms or Output::attributes
		/// @param name The name of the location
		/// @return Iterator to the vector
		Vec<ShaderVarPragma>::iterator findShaderVar(Vec<ShaderVarPragma>& vec, const std::string& name) const;

		/// Searches inside the Output::attributes or Output::trffbVaryings vectors
		/// @param vec Output::uniforms or Output::trffbVaryings
		/// @param what The name of the varying or attrib
		/// @return Iterator to the vector
		template<typename Type>
		typename Vec<Type>::const_iterator findNamed(const Vec<Type>& vec, const std::string& what) const;

		void printSourceLines() const;  ///< For debugging
		void printShaderVars() const;  ///< For debugging
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline ShaderPrePreprocessor::Pragma::Pragma(const std::string& definedInFile_, int definedInLine_):
	definedInFile(definedInFile_),
	definedInLine(definedInLine_)
{}


inline ShaderPrePreprocessor::ShaderVarPragma::ShaderVarPragma(const std::string& definedInFile_, int definedInLine_,
                                                               const std::string& name_, uint customLoc_):
	Pragma(definedInFile_, definedInLine_),
	name(name_),
	customLoc(customLoc_)
{}


inline ShaderPrePreprocessor::TrffbVaryingPragma::TrffbVaryingPragma(const std::string& definedInFile_,
                                                                     int definedInLine_,
                                                                     const std::string& name_):
	Pragma(definedInFile_, definedInLine_),
	name(name_)
{}


template<typename Type>
typename Vec<Type>::const_iterator ShaderPrePreprocessor::findNamed(const Vec<Type>& vec, const std::string& what) const
{
	typename Vec<Type>::const_iterator it = vec.begin();
	while(it != vec.end() && it->name != what)
	{
		++it;
	}
	return it;
}


#endif
