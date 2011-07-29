#ifndef BUILDIN_MATERIAL_VARIABLE_H
#define BUILDIN_MATERIAL_VARIABLE_H

#include "MaterialVariable.h"
#include "Util/ConstCharPtrHashMap.h"
#include <boost/array.hpp>


/// XXX
class BuildinMaterialVariable: public MaterialVariable
{
	public:
		/// Standard attribute variables that are acceptable inside the @ref
		/// ShaderProg
		enum BuildinVariable
		{
			// Attributes
			BV_POSITION,
			BV_TANGENT,
			BV_NORMAL,
			BV_TEX_COORDS,
			// Uniforms
			// Matrices
			BV_MODEL_MAT,
			BV_VIEW_MAT,
			BV_PROJECTION_MAT,
			BV_MODELVIEW_MAT,
			BV_VIEWPROJECTION_MAT,
			BV_NORMAL_MAT,
			BV_MODELVIEWPROJECTION_MAT,
			// FAIs (for materials in blending stage)
			BV_MS_NORMAL_FAI,
			BV_MS_DIFFUSE_FAI,
			BV_MS_SPECULAR_FAI,
			BV_MS_DEPTH_FAI,
			BV_IS_FAI,
			BV_PPS_PRE_PASS_FAI,
			BV_PPS_POST_PASS_FAI,
			// Other
			BV_RENDERER_SIZE,
			BV_SCENE_AMBIENT_COLOR,
			BV_BLURRING,
			// num
			BV_NUM ///< The number of all variables
		};

		BuildinMaterialVariable(const ShaderProgramVariable* cpSProgVar,
			const ShaderProgramVariable* dpSProgVar, const char* name);

		GETTER_R_BY_VAL(BuildinVariable, var, getVariableEnum)

		static bool isBuildin(const char* name, BuildinVariable* var = NULL,
			GLenum* dataType = NULL);

	private:
		/// Given a name of a variable find its BuildinVariable enum
		static ConstCharPtrHashMap<BuildinVariable>::Type buildinNameToEnum;

		/// Given a BuildinVariable enum it gives the GL type
		static boost::unordered_map<BuildinVariable, GLenum> buildinToGlType;

		BuildinVariable var;
};


#endif
