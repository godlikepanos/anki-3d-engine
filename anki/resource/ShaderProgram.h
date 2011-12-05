#ifndef ANKI_RESOURCE_SHADER_PROGRAM_H
#define ANKI_RESOURCE_SHADER_PROGRAM_H

#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/util/Assert.h"
#include "anki/resource/ShaderProgramUniformVariable.h"
#include "anki/resource/ShaderProgramAttributeVariable.h"
#include "anki/gl/GlStateMachine.h"
#include "anki/core/Globals.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <GL/glew.h>
#include <vector>
#include <boost/noncopyable.hpp>


namespace anki {


/// Shader program resource
///
/// Shader program. Combines a fragment and a vertex shader. Every shader
/// program consist of one OpenGL ID, a vector of uniform variables and a
/// vector of attribute variables. Every variable is a struct that contains
/// the variable's name, location, OpenGL data type and if it is a uniform or
/// an attribute var.
class ShaderProgram: public boost::noncopyable
{
public:
	typedef boost::ptr_vector<ShaderProgramVariable> VariablesContainer;
	typedef std::vector<ShaderProgramUniformVariable*>
		UniformVariablesContainer;
	typedef std::vector<ShaderProgramAttributeVariable*>
		AttributeVariablesContainer;

	ShaderProgram()
		: glId(UNINITIALIZED_ID)
	{}

	~ShaderProgram();

	/// @name Accessors
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isInitialized());
		return glId;
	}

	/// Get all variables container
	const VariablesContainer& getVariables() const
	{
		return vars;
	}

	const UniformVariablesContainer& getUniformVariables() const
	{
		return unis;
	}

	const AttributeVariablesContainer& getAttributeVariables() const
	{
		return attribs;
	}
	/// @}

	/// Resource load
	void load(const char* filename);

	/// Bind the shader program
	void bind() const
	{
		ANKI_ASSERT(isInitialized());
		GlStateMachineSingleton::get().useShaderProg(glId);
	}

	/// @name Variable finders
	/// Used to find and return the variable. They throw exception if
	/// variable not found so ask if the variable with that name exists
	/// prior using any of these
	/// @{
	const ShaderProgramVariable& findVariableByName(const char* varName) const;
	const ShaderProgramUniformVariable& findUniformVariableByName(
		const char* varName) const;
	const ShaderProgramAttributeVariable& findAttributeVariableByName(
		const char* varName) const;
	/// @}

	/// @name Check for variable existance
	/// @{
	bool variableExists(const char* varName) const;
	bool uniformVariableExists(const char* varName) const;
	bool attributeVariableExists(const char* varName) const;
	/// @}

	/// Used by @ref Material and @ref Renderer to create custom shaders in
	/// the cache
	/// @param sProgFPathName The file pathname of the shader prog
	/// @param preAppendedSrcCode The source code we want to write on top
	/// of the shader prog
	/// @param newFNamePrefix The prefix of the new shader prog
	/// @return The file pathname of the new shader prog. Its
	/// $HOME/.anki/cache/newFNamePrefix_fName
	static std::string createSrcCodeToCache(const char* sProgFPathName,
		const char* preAppendedSrcCode);

	/// For sorting
	bool operator<(const ShaderProgram& b) const
	{
		return glId < b.glId;
	}

	/// For debugging
	friend std::ostream& operator<<(std::ostream& s,
		const ShaderProgram& x);

private:
	typedef ConstCharPtrHashMap<ShaderProgramVariable*>::Type
		NameToVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramUniformVariable*>::Type
		NameToUniVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramAttributeVariable*>::Type
		NameToAttribVarHashMap;

	static const GLuint UNINITIALIZED_ID = -1;

	std::string rsrcFilename;
	GLuint glId; ///< The OpenGL ID of the shader program
	GLuint vertShaderGlId; ///< Vertex shader OpenGL id
	GLuint tcShaderGlId; ///< Tessellation control shader OpenGL id
	GLuint teShaderGlId; ///< Tessellation eval shader OpenGL id
	GLuint geomShaderGlId; ///< Geometry shader OpenGL id
	GLuint fragShaderGlId; ///< Fragment shader OpenGL id

	/// Shader source that is used in ALL shader programs
	static const char* stdSourceCode;

	/// @name Containers
	/// @{
	VariablesContainer vars; ///< All the vars. Does garbage collection
	UniformVariablesContainer unis;
	AttributeVariablesContainer attribs;

	NameToVarHashMap nameToVar; ///< Variable searching
	NameToUniVarHashMap nameToUniVar; ///< Uniform searching
	NameToAttribVarHashMap nameToAttribVar; ///< Attribute searching
	/// @}

	/// Query the driver to get the vars. After the linking of the shader
	/// prog is done gather all the vars in custom containers
	void getUniAndAttribVars();

	/// Create and compile shader
	/// @return The shader's OpenGL id
	/// @exception Exception
	uint createAndCompileShader(const char* sourceCode,
		const char* preproc, int type) const;

	/// Link the shader program
	/// @exception Exception
	void link() const;

	/// Returns true if the class points to a valid GL ID
	bool isInitialized() const
	{
		return glId != UNINITIALIZED_ID;
	}
}; 


} // end namespace


#endif
