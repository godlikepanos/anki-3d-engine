#include "BuildinMaterialVariable.h"
#include "Util/Exception.h"
#include "Util/Assert.h"
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

ConstCharPtrHashMap<BuildinMaterialVariable::BuildinVariable>::Type
	BuildinMaterialVariable::buildinNameToEnum = boost::assign::map_list_of
	("position", BV_POSITION)
	("tangent", BV_TANGENT)
	("normal", BV_NORMAL)
	("texCoords", BV_TEX_COORDS)
	("modelMat", BV_MODEL_MAT)
	("viewMat", BV_VIEW_MAT)
	("projectionMat", BV_PROJECTION_MAT)
	("modelViewMat", BV_MODELVIEW_MAT)
	("viewProjectionMat", BV_VIEWPROJECTION_MAT)
	("normalMat", BV_NORMAL_MAT)
	("modelViewProjectionMat", BV_MODELVIEWPROJECTION_MAT)
	("msNormalFai", BV_MS_NORMAL_FAI)
	("msDiffuseFai", BV_MS_DIFFUSE_FAI)
	("msSpecularFai", BV_MS_SPECULAR_FAI)
	("msDepthFai", BV_MS_DEPTH_FAI)
	("isFai", BV_IS_FAI)
	("ppsPrePassFai", BV_PPS_PRE_PASS_FAI)
	("ppsPostPassFai", BV_PPS_POST_PASS_FAI)
	("rendererSize", BV_RENDERER_SIZE)
	("sceneAmbientColor", BV_SCENE_AMBIENT_COLOR)
	("blurring", BV_BLURRING);


boost::unordered_map<BuildinMaterialVariable::BuildinVariable, GLenum>
	BuildinMaterialVariable::buildinToGlType = boost::assign::map_list_of
	(BV_POSITION, GL_FLOAT_VEC3)
	(BV_TANGENT, GL_FLOAT_VEC4)
	(BV_NORMAL, GL_FLOAT_VEC3)
	(BV_TEX_COORDS, GL_FLOAT_VEC2)
	(BV_MODEL_MAT, GL_FLOAT_MAT4)
	(BV_VIEW_MAT, GL_FLOAT_MAT4)
	(BV_PROJECTION_MAT, GL_FLOAT_MAT4)
	(BV_PROJECTION_MAT, GL_FLOAT_MAT4)
	(BV_VIEWPROJECTION_MAT, GL_FLOAT_MAT4)
	(BV_NORMAL_MAT, GL_FLOAT_MAT3)
	(BV_MODELVIEWPROJECTION_MAT, GL_FLOAT_MAT4)
	(BV_MS_NORMAL_FAI, GL_SAMPLER_2D)
	(BV_MS_DIFFUSE_FAI, GL_SAMPLER_2D)
	(BV_MS_SPECULAR_FAI, GL_SAMPLER_2D)
	(BV_MS_DEPTH_FAI, GL_SAMPLER_2D)
	(BV_IS_FAI, GL_SAMPLER_2D)
	(BV_PPS_PRE_PASS_FAI, GL_SAMPLER_2D)
	(BV_PPS_POST_PASS_FAI, GL_SAMPLER_2D)
	(BV_RENDERER_SIZE, GL_FLOAT_VEC2)
	(BV_SCENE_AMBIENT_COLOR, GL_FLOAT_VEC3)
	(BV_BLURRING, GL_FLOAT);


//==============================================================================
// Constructor                                                                 =
//==============================================================================
BuildinMaterialVariable::BuildinMaterialVariable(
	const ShaderProgramVariable* cpSProgVar,
	const ShaderProgramVariable* dpSProgVar,
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
	ConstCharPtrHashMap<BuildinVariable>::Type::const_iterator it =
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
		boost::unordered_map<BuildinVariable, GLenum>::const_iterator it2 =
			buildinToGlType.find(it->second);

		ASSERT(it2 != buildinToGlType.end());

		*dataType = it2->second;
	}

	return true;
}
