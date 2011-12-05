#ifndef ANKI_RESOURCE_MATERIAL_VARIABLE_H
#define ANKI_RESOURCE_MATERIAL_VARIABLE_H

#include "anki/resource/ShaderProgramUniformVariable.h"
#include "anki/resource/MaterialCommon.h"
#include "anki/math/Math.h"
#include "anki/resource/Resource.h"
#include "anki/resource/Texture.h" // For one of the constructors
#include "anki/util/Assert.h"
#include <GL/glew.h>
#include <string>
#include <boost/variant.hpp>


namespace anki {


/// Holds the shader variables. Its a container for shader program variables
/// that share the same name
class MaterialVariable
{
public:
	/// The data union (limited to a few types at the moment)
	typedef boost::variant<float, Vec2, Vec3, Vec4, Mat3,
		Mat4, TextureResourcePointer> Variant;

	/// Given a pair of pass and level it returns a pointer to a
	/// shader program uniform variable. The pointer may be null
	typedef PassLevelHashMap<const ShaderProgramUniformVariable*>::Type
		PassLevelToShaderProgramUniformVariableHashMap;

	/// @name Constructors & destructor
	/// @{

	/// For build-ins
	template<typename Type>
	MaterialVariable(
		const char* shaderProgVarName,
		const PassLevelToShaderProgramHashMap& sProgs)
		: initialized(false)
	{
		init(shaderProgVarName, sProgs);
		data = Type();
	}

	/// For user defined
	template<typename Type>
	MaterialVariable(
		const char* shaderProgVarName,
		const PassLevelToShaderProgramHashMap& sProgs,
		const Type& val)
		: initialized(true)
	{
		init(shaderProgVarName, sProgs);
		data = val;
	}
	/// @}
		
	/// @name Accessors
	/// @{
	const Variant& getVariant() const
	{
		return data;
	}

	/// XXX
	const ShaderProgramUniformVariable& getShaderProgramUniformVariable(
		const PassLevelKey& key) const
	{
		ANKI_ASSERT(inPass(key));
		const ShaderProgramUniformVariable* var = sProgVars.at(key);
		return *var;
	}

	/// Check if the shader program of the given pass and level needs
	/// contains this variable or not
	bool inPass(const PassLevelKey& key) const
	{
		return sProgVars.find(key) != sProgVars.end();
	}

	/// Get the GL data type of all the shader program variables
	GLenum getGlDataType() const
	{
		return oneSProgVar->getGlDataType();
	}

	/// Get the name of all the shader program variables
	const std::string& getName() const
	{
		return oneSProgVar->getName();
	}

	bool getInitialized() const
	{
		return initialized;
	}
	/// @}

private:
	bool initialized;
	PassLevelToShaderProgramUniformVariableHashMap sProgVars;
	Variant data;

	/// Keep one ShaderProgramVariable here for easy access of the common
	/// variable stuff like name or GL data type etc
	const ShaderProgramUniformVariable* oneSProgVar;

	/// Common constructor code
	void init(const char* shaderProgVarName,
		const PassLevelToShaderProgramHashMap& shaderProgsArr);
};


/// Declaration for specialized XXX
template<>
MaterialVariable::MaterialVariable(
	const char* shaderProgVarName,
	const PassLevelToShaderProgramHashMap& sProgs,
	const std::string& val);


} // end namespace


#endif
