#include "MaterialVariable.h"
#include "SProgVar.h"
#include "Util/Assert.h"
#include "Util/Exception.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialVariable::MaterialVariable(Type type_, const SProgVar* cpSProgVar_,
	const SProgVar* dpSProgVar_)
:	type(type_),
 	cpSProgVar(cpSProgVar_),
 	dpSProgVar(dpSProgVar_)
{
	// Sanity checks
	if(cpSProgVar && dpSProgVar)
	{
		if(cpSProgVar->getName() != dpSProgVar->getName() ||
			cpSProgVar->getType() != dpSProgVar->getType() ||
			cpSProgVar->getGlDataType() != dpSProgVar->getGlDataType())
		{
			throw EXCEPTION("Incompatible shader program variables");
		}
	}

	if(!cpSProgVar && !dpSProgVar)
	{
		throw EXCEPTION("Both variables NULL");
	}
}


//==============================================================================
// getColorPassShaderProgramVariable                                           =
//==============================================================================
const SProgVar& MaterialVariable::getColorPassShaderProgramVariable() const
{
	ASSERT(isColorPass());
	return *cpSProgVar;
}


//==============================================================================
// getDepthPassShaderProgramVariable                                           =
//==============================================================================
const SProgVar& MaterialVariable::getDepthPassShaderProgramVariable() const
{
	ASSERT(isDepthPass());
	return *dpSProgVar;
}


//==============================================================================
// getGlDataType                                                               =
//==============================================================================
GLenum MaterialVariable::getGlDataType() const
{
	if(isColorPass())
	{
		return cpSProgVar->getGlDataType();
	}
	else
	{
		return dpSProgVar->getGlDataType();
	}
}


//==============================================================================
// getName                                                                     =
//==============================================================================
const std::string& MaterialVariable::getName() const
{
	if(isColorPass())
	{
		return cpSProgVar->getName();
	}
	else
	{
		return dpSProgVar->getName();
	}
}
