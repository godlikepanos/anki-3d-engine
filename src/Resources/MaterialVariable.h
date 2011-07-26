#ifndef MATERIAL_VARIABLE_H
#define MATERIAL_VARIABLE_H

#include "Util/Accessors.h"
#include <GL/glew.h>
#include <string>


class SProgVar;


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
		MaterialVariable(Type type, const SProgVar* cpSProgVar,
			const SProgVar* dpSProgVar);

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(Type, type, getType)
		const SProgVar& getColorPassShaderProgramVariable() const;
		const SProgVar& getDepthPassShaderProgramVariable() const;
		/// Applies to the color pass shader program
		bool isColorPass() const {return cpSProgVar != NULL;}
		/// Applies to the depth pass shader program
		bool isDepthPass() const {return dpSProgVar != NULL;}
		GLenum getGlDataType() const;
		const std::string& getName() const;
		/// @}

	private:
		Type type;
		const SProgVar* cpSProgVar; ///< The color pass shader program variable
		const SProgVar* dpSProgVar; ///< The depth pass shader program variable
};


#endif
