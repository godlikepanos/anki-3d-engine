#ifndef SHADER_PROG_H
#define SHADER_PROG_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <GL/glew.h>
#include <limits>
#include "Util/CharPtrHashMap.h"
#include "Util/Assert.h"
#include "SProgUniVar.h"
#include "SProgAttribVar.h"
#include "Util/Vec.h"
#include "GfxApi/GlStateMachine.h"
#include "Core/Globals.h"


/// Shader program @ref Resource
///
/// Shader program. Combines a fragment and a vertex shader. Every shader program consist of one OpenGL ID, a vector of
/// uniform variables and a vector of attribute variables. Every variable is a struct that contains the variable's name,
/// location, OpenGL data type and if it is a uniform or an attribute var.
class ShaderProg
{
	//==================================================================================================================
	// Public                                                                  =
	//==================================================================================================================
	public:
		ShaderProg();
		~ShaderProg() {/** @todo add code */}

		/// Resource load
		void load(const char* filename);

		/// Accessor to glId
		GLuint getGlId() const;

		/// Bind the shader program
		void bind() const;
		
		/// Unbind all shader programs
		static void unbind() {glUseProgram(0);}

		/// Query the GL driver for the current shader program GL ID
		/// @return Shader program GL id
		static uint getCurrentProgramGlId();

		/// Accessor to uniform vars vector
		const boost::ptr_vector<SProgUniVar>& getUniVars() const {return uniVars;}

		/// Accessor to attribute vars vector
		const boost::ptr_vector<SProgAttribVar>& getAttribVars() const {return attribVars;}

		/// Find uniform variable. On failure it throws an exception so use @ref uniVarExists to check if var exists
		/// @param varName The name of the var
		/// @return It returns a uniform variable
		/// @exception Exception
		const SProgUniVar* findUniVar(const char* varName) const;

		/// Find Attribute variable
		/// @see findUniVar
		const SProgAttribVar* findAttribVar(const char* varName) const;

		/// Uniform variable exits
		/// @return True if uniform variable exits
		bool uniVarExists(const char* varName) const;

		/// Attribute variable exits
		/// @return True if attribute variable exits
		bool attribVarExists(const char* varName) const;

		/// Used by @ref Material and @ref Renderer to create custom shaders in the cache
		/// @param sProgFPathName The file pathname of the shader prog
		/// @param preAppendedSrcCode The source code we want to write on top of the shader prog
		/// @param newFNamePrefix The prefix of the new shader prog
		/// @return The file pathname of the new shader prog. Its $HOME/.anki/cache/newFNamePrefix_fName
		static std::string createSrcCodeToCache(const char* sProgFPathName, const char* preAppendedSrcCode);

		/// Relink the program. Used in transform feedback
		void relink() const {link();}

		/// For debuging
		std::string getShaderInfoString() const;

	//==================================================================================================================
	// Private                                                                 =
	//==================================================================================================================
	private:
		/// Uniform variable name to variable iterator
		typedef CharPtrHashMap<SProgUniVar*>::const_iterator NameToSProgUniVarIterator;
		/// Attribute variable name to variable iterator
		typedef CharPtrHashMap<SProgAttribVar*>::const_iterator NameToSProgAttribVarIterator;

		std::string rsrcFilename;
		GLuint glId; ///< The OpenGL ID of the shader program
		GLuint vertShaderGlId; ///< Vertex shader OpenGL id
		GLuint geomShaderGlId; ///< Geometry shader OpenGL id
		GLuint fragShaderGlId; ///< Fragment shader OpenGL id
		static std::string stdSourceCode; ///< Shader source that is used in ALL shader programs
		boost::ptr_vector<SProgUniVar> uniVars; ///< All the uniform variables
		boost::ptr_vector<SProgAttribVar> attribVars; ///< All the attribute variables
		CharPtrHashMap<SProgUniVar*> uniNameToVar;  ///< A UnorderedMap for fast variable searching
		CharPtrHashMap<SProgAttribVar*> attribNameToVar; ///< @see uniNameToVar

		/// Query the driver to get the vars. After the linking of the shader prog is done gather all the vars in
		/// custom containers
		void getUniAndAttribVars();

		/// Uses glBindAttribLocation for every parser attrib location
		/// @exception Exception
		void bindCustomAttribLocs(const class ShaderPrePreprocessor& pars) const;

		/// Create and compile shader
		/// @return The shader's OpenGL id
		/// @exception Exception
		uint createAndCompileShader(const char* sourceCode, const char* preproc, int type) const;

		/// Link the shader program
		/// @exception Exception
		void link() const;
}; 


//==============================================================================
// Inlines                                                                     =
//==============================================================================

inline ShaderProg::ShaderProg():
	glId(std::numeric_limits<uint>::max())
{}


inline GLuint ShaderProg::getGlId() const
{
	ASSERT(glId != std::numeric_limits<uint>::max());
	return glId;
}


inline void ShaderProg::bind() const
{
	ASSERT(glId != std::numeric_limits<uint>::max());
	GlStateMachineSingleton::getInstance().useShaderProg(glId);
}


inline uint ShaderProg::getCurrentProgramGlId()
{
	int i;
	glGetIntegerv(GL_CURRENT_PROGRAM, &i);
	return i;
}


#endif
