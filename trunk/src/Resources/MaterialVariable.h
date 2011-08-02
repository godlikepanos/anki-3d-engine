#ifndef MATERIAL_VARIABLE_H
#define MATERIAL_VARIABLE_H

#include "Util/Accessors.h"
#include <GL/glew.h>
#include <string>


class ShaderProgramVariable;


/// XXX
class MaterialVariable
{
	public:
		/// The type
		enum Type
		{
			T_USER,
			T_BUILDIN,
			T_NUM
		};

		/// XXX
		MaterialVariable(Type type, const ShaderProgramVariable* cpSProgVar,
			const ShaderProgramVariable* dpSProgVar);

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(Type, type, getType)
		/// @exception If the material variable is for depth only
		const ShaderProgramVariable& getColorPassShaderProgramVariable() const;
		/// @exception If the material variable is for color only
		const ShaderProgramVariable& getDepthPassShaderProgramVariable() const;
		/// Applies to the color pass shader program
		bool isColorPass() const {return cpSProgVar != NULL;}
		/// Applies to the depth pass shader program
		bool isDepthPass() const {return dpSProgVar != NULL;}
		GLenum getGlDataType() const;
		const std::string& getName() const;
		/// @}

	private:
		Type type;
		/// The color pass shader program variable
		const ShaderProgramVariable* cpSProgVar;
		/// The depth pass shader program variable
		const ShaderProgramVariable* dpSProgVar;
};


#endif
