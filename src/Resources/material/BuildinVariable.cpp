#include "BuildinVariable.h"
#include "Util/Exception.h"
#include "Util/Assert.h"
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


namespace material {


//==============================================================================
// Statics                                                                     =
//==============================================================================

ConstCharPtrHashMap<BuildinVariable::BuildinEnum>::Type
	BuildinVariable::buildinNameToEnum = boost::assign::map_list_of
	("position", POSITION)
	("tangent", TANGENT)
	("normal", NORMAL)
	("texCoords", TEX_COORDS)
	("modelMat", MODEL_MAT)
	("viewMat", VIEW_MAT)
	("projectionMat", PROJECTION_MAT)
	("modelViewMat", MODELVIEW_MAT)
	("viewProjectionMat", VIEWPROJECTION_MAT)
	("normalMat", NORMAL_MAT)
	("modelViewProjectionMat", MODELVIEWPROJECTION_MAT)
	("msNormalFai", MS_NORMAL_FAI)
	("msDiffuseFai", MS_DIFFUSE_FAI)
	("msSpecularFai", MS_SPECULAR_FAI)
	("msDepthFai", MS_DEPTH_FAI)
	("isFai", IS_FAI)
	("ppsPrePassFai", PPS_PRE_PASS_FAI)
	("ppsPostPassFai", PPS_POST_PASS_FAI)
	("rendererSize", RENDERER_SIZE)
	("sceneAmbientColor", SCENE_AMBIENT_COLOR)
	("blurring", BLURRING);


boost::unordered_map<BuildinVariable::BuildinEnum, GLenum>
	BuildinVariable::buildinToGlType = boost::assign::map_list_of
	(POSITION, GL_FLOAT_VEC3)
	(TANGENT, GL_FLOAT_VEC4)
	(NORMAL, GL_FLOAT_VEC3)
	(TEX_COORDS, GL_FLOAT_VEC2)
	(MODEL_MAT, GL_FLOAT_MAT4)
	(VIEW_MAT, GL_FLOAT_MAT4)
	(PROJECTION_MAT, GL_FLOAT_MAT4)
	(PROJECTION_MAT, GL_FLOAT_MAT4)
	(VIEWPROJECTION_MAT, GL_FLOAT_MAT4)
	(NORMAL_MAT, GL_FLOAT_MAT3)
	(MODELVIEWPROJECTION_MAT, GL_FLOAT_MAT4)
	(MS_NORMAL_FAI, GL_SAMPLER_2D)
	(MS_DIFFUSE_FAI, GL_SAMPLER_2D)
	(MS_SPECULAR_FAI, GL_SAMPLER_2D)
	(MS_DEPTH_FAI, GL_SAMPLER_2D)
	(IS_FAI, GL_SAMPLER_2D)
	(PPS_PRE_PASS_FAI, GL_SAMPLER_2D)
	(PPS_POST_PASS_FAI, GL_SAMPLER_2D)
	(RENDERER_SIZE, GL_FLOAT_VEC2)
	(SCENE_AMBIENT_COLOR, GL_FLOAT_VEC3)
	(BLURRING, GL_FLOAT);


//==============================================================================
// Constructor                                                                 =
//==============================================================================
BuildinVariable::BuildinVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr)
:	Variable(BUILDIN, shaderProgVarName, shaderProgsArr),
 	bEnum(BUILDINS_NUM)
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
bool BuildinVariable::isBuildin(const char* name,
	BuildinEnum* var, GLenum* dataType)
{
	ConstCharPtrHashMap<BuildinEnum>::Type::const_iterator it =
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
		boost::unordered_map<BuildinEnum, GLenum>::const_iterator it2 =
			buildinToGlType.find(it->second);

		ASSERT(it2 != buildinToGlType.end());

		*dataType = it2->second;
	}

	return true;
}


} // end namespace
