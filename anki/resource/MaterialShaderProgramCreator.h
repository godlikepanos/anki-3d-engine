#ifndef ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H
#define ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H

#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/util/scanner/Forward.h"
#include <GL/glew.h>
#include <vector>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


/// Creator of shader programs. This class parses between
/// <shaderProgam></shaderProgram> located inside a <material></material>
/// and creates the source of a custom program.
class MaterialShaderProgramCreator
{
	public:
		MaterialShaderProgramCreator(const boost::property_tree::ptree& pt);
		~MaterialShaderProgramCreator();

		/// Get the shader program source code. This is the one and only public
		/// member
		const std::string& getShaderProgramSource() const
		{
			return source;
		}

	private:
		//======================================================================
		// Nested                                                              =
		//======================================================================

		/// Function argument data call type
		enum ArgQualifier
		{
			AQ_IN,
			AQ_OUT,
			AQ_INOUT
		};

		/// Function argument definition
		struct ArgDefinition
		{
			ArgQualifier argQualifier;
			std::string argQualifierTxt;
			GLenum dataType;
			std::string dataTypeTxt;
			std::string name;
		};

		/// Function definition. It contains information about a function (eg
		/// return type and the arguments)
		struct FuncDefinition
		{
			std::string name; ///< Function name
			boost::ptr_vector<ArgDefinition> argDefinitions;
			GLenum returnDataType;
			std::string returnDataTypeTxt;
		};

		//======================================================================
		// Members                                                             =
		//======================================================================

		/// Go from enum GL_SOMETHING to "GL_SOMETHING" string
		static boost::unordered_map<GLenum, const char*> glTypeToTxt;

		/// Go from "GL_SOMETHING" string to GL_SOMETHING enum
		static ConstCharPtrHashMap<GLenum>::Type txtToGlType;

		/// Go from enum AQ_SOMETHING to "AQ_SOMETHING" string
		static boost::unordered_map<ArgQualifier, const char*>
			argQualifierToTxt;

		/// Go from "AQ_SOMETHING" string to AQ_SOMETHING enum
		static ConstCharPtrHashMap<ArgQualifier>::Type txtToArgQualifier;

		/// This holds the varyings that come from the vertex shader to the
		/// fragment. These varyings can be used in the ins
		/// - vTexCoords
		/// - vNormal
		/// - vTangent
		/// - vTangentW
		/// - vVertPosViewSpace
		static ConstCharPtrHashMap<GLenum>::Type varyingNameToGlType;

		/// @name Using attribute flag
		/// Keep a few flags here to set a few defines in the shader program
		/// source. The parseInputTag sets them
		/// @{
		bool usingTexCoordsAttrib;
		bool usingNormalAttrib;
		bool usingTangentAttrib;
		/// @}

		/// Container that holds the function definitions
		boost::ptr_vector<FuncDefinition> funcDefs;

		/// Go from function name to function definition
		ConstCharPtrHashMap<FuncDefinition*>::Type funcNameToDef;

		/// The lines of the shader program source
		std::vector<std::string> srcLines;

		std::string source; ///< Shader program final source

		//======================================================================
		// Methods                                                             =
		//======================================================================

		/// Parses a glsl file for function definitions. It appends the
		/// funcDefs container with new function definitions
		void parseShaderFileForFunctionDefinitions(const char* filename);

		/// Used by parseShaderFileForFunctionDefinitions to skip preprocessor
		/// definitions. Takes into account the backslashes. For example for
		/// @code
		/// #define ANKI_RESOURCE_lala \\ _
		/// 	10
		/// @endcode
		/// it skips from define to 10
		static void parseUntilNewline(scanner::Scanner& scanner);

		/// It being used by parseShaderFileForFunctionDefinitions and it
		/// skips until newline after a '#' found. It takes into account the
		/// back slashes that the preprocessor may have
		static void getNextTokenAndSkipNewlines(scanner::Scanner& scanner);

		/// Used for shorting vectors of strings. Used in std::sort
		static bool compareStrings(const std::string& a, const std::string& b);

		/// Parse what is within the
		/// @code <shaderProgram></shaderProgram> @endcode
		void parseShaderProgramTag(const boost::property_tree::ptree& pt);

		/// Parse what is within the @code <input></input> @endcode
		void parseInputTag(const boost::property_tree::ptree& pt,
			std::string& line);

		/// Parse what is within the @code <operation></operation> @endcode
		void parseOperatorTag(const boost::property_tree::ptree& pt);
};


} // end namespace


#endif
