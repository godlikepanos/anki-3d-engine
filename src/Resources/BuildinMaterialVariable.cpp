#include "BuildinMaterialVariable.h"
#include "Util/Exception.h"
#include <cstring>
#include <boost/lexical_cast.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

boost::array<BuildinMaterialVariable::BuildinVarInfo,
	BuildinMaterialVariable::BV_NUM> BuildinMaterialVariable::infos =
{{
	{"position", GL_FLOAT_VEC3},
	{"tangent", GL_FLOAT_VEC4},
	{"normal", GL_FLOAT_VEC3},
	{"texCoords", GL_FLOAT_VEC2},
	{"modelMat", GL_FLOAT_MAT4},
	{"viewMat", GL_FLOAT_MAT4},
	{"projectionMat", GL_FLOAT_MAT4},
	{"modelViewMat", GL_FLOAT_MAT4},
	{"viewProjectionMat", GL_FLOAT_MAT4},
	{"normalMat", GL_FLOAT_MAT3},
	{"modelViewProjectionMat", GL_FLOAT_MAT4},
	{"msNormalFai", GL_SAMPLER_2D},
	{"msDiffuseFai", GL_SAMPLER_2D},
	{"msSpecularFai", GL_SAMPLER_2D},
	{"msDepthFai", GL_SAMPLER_2D},
	{"isFai", GL_SAMPLER_2D},
	{"ppsPrePassFai", GL_SAMPLER_2D},
	{"ppsPostPassFai", GL_SAMPLER_2D},
	{"rendererSize", GL_FLOAT_VEC2},
	{"sceneAmbientColor", GL_FLOAT_VEC3},
	{"blurring", GL_FLOAT},
}};


//==============================================================================
// Constructor                                                                 =
//==============================================================================
BuildinMaterialVariable::BuildinMaterialVariable(
	const SProgVar* cpSProgVar, const SProgVar* dpSProgVar,
	const char* name)
:	MaterialVariable(T_BUILDIN, cpSProgVar, dpSProgVar),
 	var(BV_NUM)
{
	GLenum dataType;
	// Sanity checks
	if(!isBuildin(name, &var, &dataType))
	{
		throw EXCEPTION("The variable is not buildin: " + name);
	}

	if(dataType != getGlDataType())
	{
		throw EXCEPTION("The buildin variable \"" + name +
			"\" sould be of type: " +
			boost::lexical_cast<std::string>(dataType));
	}
}


//==============================================================================
// isBuildin                                                                   =
//==============================================================================
bool BuildinMaterialVariable::isBuildin(const char* name,
	BuildinVariable* var, GLenum* dataType)
{
	for(uint i = 0; i < BV_NUM; i++)
	{
		if(!strcmp(infos[i].name, name))
		{
			if(var)
			{
				*var = static_cast<BuildinVariable>(i);
			}

			if(dataType)
			{
				*dataType = infos[i].dataType;
			}
			return true;
		}
	}

	return false;
}
