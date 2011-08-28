#include "MaterialBuildinVariable.h"
#include "util/Exception.h"
#include "util/Assert.h"
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

MaterialBuildinVariable::StrToMatchingVariable
	MaterialBuildinVariable::strToMatchingVariable = boost::assign::map_list_of
	("position", MV_POSITION)
	("tangent", MV_TANGENT)
	("normal", MV_NORMAL)
	("texCoords", MV_TEX_COORDS)
	("modelMat", MV_MODEL_MAT)
	("viewMat", MV_VIEW_MAT)
	("projectionMat", MV_PROJECTION_MAT)
	("modelViewMat", MV_MODELVIEW_MAT)
	("viewProjectionMat", MV_VIEWPROJECTION_MAT)
	("normalMat", MV_NORMAL_MAT)
	("modelViewProjectionMat", MV_MODELVIEWPROJECTION_MAT)
	("msNormalFai", MV_MS_NORMAL_FAI)
	("msDiffuseFai", MV_MS_DIFFUSE_FAI)
	("msSpecularFai", MV_MS_SPECULAR_FAI)
	("msDepthFai", MV_MS_DEPTH_FAI)
	("isFai", MV_IS_FAI)
	("ppsPrePassFai", MV_PPS_PRE_PASS_FAI)
	("ppsPostPassFai", MV_PPS_POST_PASS_FAI)
	("rendererSize", MV_RENDERER_SIZE)
	("sceneAmbientColor", MV_SCENE_AMBIENT_COLOR)
	("blurring", MV_BLURRING);


MaterialBuildinVariable::MatchingVariableToGlType
	MaterialBuildinVariable::matchingVariableToGlType =
	boost::assign::map_list_of
	(MV_POSITION, GL_FLOAT_VEC3)
	(MV_TANGENT, GL_FLOAT_VEC4)
	(MV_NORMAL, GL_FLOAT_VEC3)
	(MV_TEX_COORDS, GL_FLOAT_VEC2)
	(MV_MODEL_MAT, GL_FLOAT_MAT4)
	(MV_VIEW_MAT, GL_FLOAT_MAT4)
	(MV_PROJECTION_MAT, GL_FLOAT_MAT4)
	(MV_MODELVIEW_MAT, GL_FLOAT_MAT4)
	(MV_VIEWPROJECTION_MAT, GL_FLOAT_MAT4)
	(MV_NORMAL_MAT, GL_FLOAT_MAT3)
	(MV_MODELVIEWPROJECTION_MAT, GL_FLOAT_MAT4)
	(MV_MS_NORMAL_FAI, GL_SAMPLER_2D)
	(MV_MS_DIFFUSE_FAI, GL_SAMPLER_2D)
	(MV_MS_SPECULAR_FAI, GL_SAMPLER_2D)
	(MV_MS_DEPTH_FAI, GL_SAMPLER_2D)
	(MV_IS_FAI, GL_SAMPLER_2D)
	(MV_PPS_PRE_PASS_FAI, GL_SAMPLER_2D)
	(MV_PPS_POST_PASS_FAI, GL_SAMPLER_2D)
	(MV_RENDERER_SIZE, GL_FLOAT_VEC2)
	(MV_SCENE_AMBIENT_COLOR, GL_FLOAT_VEC3)
	(MV_BLURRING, GL_FLOAT);


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialBuildinVariable::MaterialBuildinVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr)
:	MaterialVariable(T_BUILDIN, shaderProgVarName, shaderProgsArr),
 	bEnum(MV_NUM)
{
	GLenum dataType;
	// Sanity checks
	if(!isBuildin(shaderProgVarName, &bEnum, &dataType))
	{
		throw EXCEPTION("The variable is not buildin: " + shaderProgVarName);
	}

	if(dataType != getGlDataType())
	{
		throw EXCEPTION("The buildin variable \"" + shaderProgVarName +
			"\" should be of type: " +
			boost::lexical_cast<std::string>(dataType));
	}
}


//==============================================================================
// isBuildin                                                                   =
//==============================================================================
bool MaterialBuildinVariable::isBuildin(const char* name,
	MatchingVariable* var, GLenum* dataType)
{
	StrToMatchingVariable::const_iterator it = strToMatchingVariable.find(name);

	if(it == strToMatchingVariable.end())
	{
		return false;
	}

	MatchingVariable mv = it->second;

	if(var)
	{
		*var = mv;
	}

	if(dataType)
	{
		MatchingVariableToGlType::const_iterator it2 =
			matchingVariableToGlType.find(mv);

		ASSERT(it2 != matchingVariableToGlType.end());

		*dataType = it2->second;
	}

	return true;
}
