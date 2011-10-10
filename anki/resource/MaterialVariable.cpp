#include "anki/resource/MaterialVariable.h"
#include "anki/resource/ShaderProgramVariable.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialVariable::MaterialVariable(
	Type type_,
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr)
:	type(type_),
 	oneSProgVar(NULL)
{
	// For all shader progs point to the variables
	for(uint i = 0; i < shaderProgsArr.size(); i++)
	{
		if(shaderProgsArr[i]->variableExists(shaderProgVarName))
		{
			sProgVars[i] = &shaderProgsArr[i]->getVariableByName(
				shaderProgVarName);

			if(!oneSProgVar)
			{
				oneSProgVar = sProgVars[i];
			}

			// Sanity check: All the sprog vars need to have same GL data type
			if(oneSProgVar->getGlDataType() != sProgVars[i]->getGlDataType() ||
				oneSProgVar->getType() != sProgVars[i]->getType())
			{
				throw EXCEPTION("Incompatible shader program variables: " +
					shaderProgVarName);
			}
		}
		else
		{
			sProgVars[i] = NULL;
		}
	}

	// Extra sanity checks
	if(!oneSProgVar)
	{
		throw EXCEPTION("Variable not found in any of the shader programs: " +
			shaderProgVarName);
	}
}
