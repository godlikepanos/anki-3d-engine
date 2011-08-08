#ifndef MATERIAL_BUILDIN_VARIABLE_H
#define MATERIAL_BUILDIN_VARIABLE_H

#include "Variable.h"
#include "Util/ConstCharPtrHashMap.h"
#include <boost/array.hpp>


namespace material {


/// XXX
class BuildinVariable: public Variable
{
	public:
		/// Standard attribute variables that are acceptable inside the
		/// ShaderProgram
		enum BuildinEnum
		{
			// Attributes
			POSITION,
			TANGENT,
			NORMAL,
			TEX_COORDS,
			// Uniforms
			// Matrices
			MODEL_MAT,
			VIEW_MAT,
			PROJECTION_MAT,
			MODELVIEW_MAT,
			VIEWPROJECTION_MAT,
			NORMAL_MAT,
			MODELVIEWPROJECTION_MAT,
			// FAIs (for materials in blending stage)
			MS_NORMAL_FAI,
			MS_DIFFUSE_FAI,
			MS_SPECULAR_FAI,
			MS_DEPTH_FAI,
			IS_FAI,
			PPS_PRE_PASS_FAI,
			PPS_POST_PASS_FAI,
			// Other
			RENDERER_SIZE,
			SCENE_AMBIENT_COLOR,
			BLURRING,
			// num
			BUILDINS_NUM ///< The number of all buildin variables
		};

		BuildinVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr);

		GETTER_R_BY_VAL(BuildinEnum, bEnum, getVariableEnum)

		static bool isBuildin(const char* name, BuildinEnum* var = NULL,
			GLenum* dataType = NULL);

	private:
		/// Given a name of a variable find its BuildinVariable enum
		static ConstCharPtrHashMap<BuildinEnum>::Type buildinNameToEnum;

		/// Given a BuildinVariable enum it gives the GL type
		static boost::unordered_map<BuildinEnum, GLenum> buildinToGlType;

		BuildinEnum bEnum;
};


} // end namespace

#endif
