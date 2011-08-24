#include "MaterialBuildinVariable.h"
#include "Util/Exception.h"
#include "Util/Assert.h"
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

ConstCharPtrHashMap<MaterialBuildinVariable::MatchingVariable>::Type
	MaterialBuildinVariable::buildinNameToEnum = boost::assign::map_list_of
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


boost::unordered_map<MaterialBuildinVariable::MatchingVariable, GLenum>
	MaterialBuildinVariable::buildinToGlType = boost::assign::map_list_of
	(MV_POSITION, GL_FLOAT_VEC3)
	(MV_TANGENT, GL_FLOAT_VEC4)
	(MV_NORMAL, GL_FLOAT_VEC3)
	(MV_TEX_COORDS, GL_FLOAT_VEC2)
	(MV_MODEL_MAT, GL_FLOAT_MAT4)
	(MV_VIEW_MAT, GL_FLOAT_MAT4)
	(MV_PROJECTION_MAT, GL_FLOAT_MAT4)
	(MV_PROJECTION_MAT, GL_FLOAT_MAT4)
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
	ConstCharPtrHashMap<MatchingVariable>::Type::const_iterator it =
		buildinNameToEnum.find(name);

	if(it == buildinNameToEnum.end())
	{
		return false;
	}

	if(var)
	{
		*var = it->second;
	}

	if(dataType)
	{
		boost::unordered_map<MatchingVariable, GLenum>::const_iterator it2 =
			buildinToGlType.find(it->second);

		ASSERT(it2 != buildinToGlType.end());

		*dataType = it2->second;
	}

	return true;
}
