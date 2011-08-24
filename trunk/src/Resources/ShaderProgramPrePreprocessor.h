#ifndef SHADER_PROGRAM_PRE_PREPROCESSOR_H
#define SHADER_PROGRAM_PRE_PREPROCESSOR_H

#include <limits>
#include "Util/Vec.h"
#include "Util/StdTypes.h"
#include "Util/Accessors.h"


namespace scanner {
class Scanner;
}


/// Helper class used for shader program loading
///
/// The class fills some of the GLSL spec deficiencies. It adds the include
/// preprocessor directive and the support to have all the shaders in the same
/// file. The file that includes all the shaders is called
/// PrePreprocessor-compatible.
///
/// The preprocessor pragmas are:
///
/// - #pragma anki start <vertexShader | geometryShader | fragmentShader>
/// - #pragma anki include "<filename>"
/// - #pragma anki transformFeedbackVarying <varName>
///
/// @note The order of the *ShaderBegins is important
class ShaderProgramPrePreprocessor
{
	public:
		/// It loads a file and parses it
		/// @param[in] filename The file to load
		/// @exception Exception
		ShaderProgramPrePreprocessor(const char* filename)
			{parseFile(filename);}

		/// Destructor does nothing
		~ShaderProgramPrePreprocessor() {}

		/// @name Accessors
		/// @{
		GETTER_R(Vec<std::string>, trffbVaryings, getTranformFeedbackVaryings)
		GETTER_R(std::string, output.vertShaderSource, getVertexShaderSource)
		GETTER_R(std::string, output.geomShaderSource, getGeometryShaderSource)
		GETTER_R(std::string, output.fragShaderSource, getFragmentShaderSource)
		/// @}

	protected:
		/// The pragma base class
		struct Pragma
		{
			std::string definedInFile;
			int definedInLine;
			Pragma(): definedInLine(-1) {}
			Pragma(const std::string& definedInFile_, int definedInLine_);
		};
		
		struct IncludePragma: Pragma
		{
			std::string filename;
		};

		struct TrffbVaryingPragma: Pragma
		{
			std::string name;

			TrffbVaryingPragma(const std::string& definedInFile_,
				int definedInLine_, const std::string& name_);
		};
	
		struct CodeBeginningPragma: Pragma
		{
			/// The line number in the PrePreprocessor-compatible
			/// file
			int globalLine;

			CodeBeginningPragma(): globalLine(-1) {}
		};

		/// The output of the class packed in this struct
		struct Output
		{
			friend class PrePreprocessor;

			/// Names and and ids for transform feedback varyings
			Vec<TrffbVaryingPragma> trffbVaryings;
			std::string vertShaderSource; ///< The vert shader source
			std::string geomShaderSource; ///< The geom shader source
			std::string fragShaderSource; ///< The frag shader source
		};

		Output output; ///< The most important variable
		Vec<std::string> trffbVaryings;
		Vec<std::string> sourceLines;  ///< The parseFileForPragmas fills this
		CodeBeginningPragma vertShaderBegins;
		CodeBeginningPragma geomShaderBegins;
		CodeBeginningPragma fragShaderBegins;

		/// Parse a PrePreprocessor formated GLSL file. Use
		/// the accessors to get the output
		/// @param filename The file to parse
		/// @exception Ecxeption
		void parseFile(const char* filename);

		/// A recursive function that parses a file for pragmas and updates
		/// the output
		/// @param filename The file to parse
		/// @param depth The #line in GLSL does not support filename so an
		/// depth it being used. It also tracks the
		/// includance depth
		/// @exception Ecxeption
		void parseFileForPragmas(const std::string& filename, int depth = 0);

		/// @todo
		void parseStartPragma(scanner::Scanner& scanner,
			const std::string& filename, uint depth,
			const Vec<std::string>& lines);

		/// @todo
		void parseIncludePragma(scanner::Scanner& scanner,
			const std::string& filename, uint depth,
			const Vec<std::string>& lines);

		/// @todo
		void parseTrffbVarying(scanner::Scanner& scanner,
			const std::string& filename, uint depth,
			const Vec<std::string>& lines);

		/// Searches inside the Output::attributes or Output::trffbVaryings
		/// vectors
		/// @param vec Output::uniforms or Output::trffbVaryings
		/// @param what The name of the varying or attrib
		/// @return Iterator to the vector
		template<typename Type>
		typename Vec<Type>::const_iterator findNamed(const Vec<Type>& vec,
			const std::string& what) const;

		void printSourceLines() const;  ///< For debugging
};


//==============================================================================
// Inlines                                                                     =
//==============================================================================

inline ShaderProgramPrePreprocessor::Pragma::Pragma(
	const std::string& definedInFile_, int definedInLine_)
:	definedInFile(definedInFile_),
	definedInLine(definedInLine_)
{}


inline ShaderProgramPrePreprocessor::TrffbVaryingPragma::TrffbVaryingPragma(
	const std::string& definedInFile_,
	int definedInLine_, const std::string& name_)
:	Pragma(definedInFile_, definedInLine_),
	name(name_)
{}


template<typename Type>
typename Vec<Type>::const_iterator ShaderProgramPrePreprocessor::findNamed(
	const Vec<Type>& vec, const std::string& what) const
{
	typename Vec<Type>::const_iterator it = vec.begin();
	while(it != vec.end() && it->name != what)
	{
		++it;
	}
	return it;
}


#endif
