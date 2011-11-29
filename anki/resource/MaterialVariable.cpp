#include "anki/resource/MaterialVariable.h"
#include "anki/resource/ShaderProgramVariable.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"


namespace anki {


//==============================================================================
MaterialVariable::MaterialVariable(
	const char* shaderProgVarName,
	const PassLevelToShaderProgramHashMap& sProgs)
:	type(T_BUILDIN)
{
	init(shaderProgVarName, sProgs);
}


//==============================================================================
template <>
MaterialVariable::MaterialVariable(
	const char* shaderProgVarName,
	const PassLevelToShaderProgramHashMap& sProgs,
	const std::string& val)
	: type(T_USER)
{
	init(shaderProgVarName, sProgs);
	ANKI_ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = TextureResourcePointer();
	boost::get<TextureResourcePointer>(data).load(val.c_str());
}


//==============================================================================
void MaterialVariable::init(const char* shaderProgVarName,
	const PassLevelToShaderProgramHashMap& sProgs)
{
	oneSProgVar = NULL;

	PassLevelToShaderProgramHashMap::const_iterator it = sProgs.begin();
	for(; it != sProgs.end(); ++it)
	{
		const ShaderProgram& sProg = *(it->second);
		const PassLevelKey& key = it->first;

		if(sProg.variableExists(shaderProgVarName))
		{
			const ShaderProgramVariable& sProgVar =
				sProg.getVariableByName(shaderProgVarName);

			sProgVars[key] = &sProgVar;

			if(!oneSProgVar)
			{
				oneSProgVar = &sProgVar;
			}

			// Sanity check: All the sprog vars need to have same GL data type
			if(oneSProgVar->getGlDataType() != sProgVar.getGlDataType() ||
				oneSProgVar->getType() != sProgVar.getType())
			{
				throw ANKI_EXCEPTION("Incompatible shader "
					"program variables: " +
					shaderProgVarName);
			}
		}
		else
		{
			sProgVars[key] = NULL;
		}
	}

	// Extra sanity checks
	if(!oneSProgVar)
	{
		throw ANKI_EXCEPTION("Variable not found in "
			"any of the shader programs: " +
			shaderProgVarName);
	}
}


} // end namespace
