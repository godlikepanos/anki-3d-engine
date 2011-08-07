#ifndef UNIFORM_SHADER_PROGRAM_VARIABLE_H
#define UNIFORM_SHADER_PROGRAM_VARIABLE_H

#include "ShaderProgramVariable.h"
#include "Math/Math.h"


class Texture;


/// Uniform shader variable
class UniformShaderProgramVariable: public ShaderProgramVariable
{
	public:
		UniformShaderProgramVariable(int loc_, const char* name_,
			GLenum glDataType_,
			const ShaderProgram& fatherSProg_);

		/// @name Set the var
		/// @{
		void set(const float f[], uint size = 1) const;
		void set(const Vec2 v2[], uint size = 1) const;
		void set(const Vec3 v3[], uint size = 1) const;
		void set(const Vec4 v4[], uint size = 1) const;
		void set(const Mat3 m3[], uint size = 1) const;
		void set(const Mat4 m4[], uint size = 1) const;
		void set(const Texture& tex, uint texUnit) const;
		/// @}
};


inline UniformShaderProgramVariable::UniformShaderProgramVariable(
	int loc_, const char* name_,
	GLenum glDataType_, const ShaderProgram& fatherSProg_)
:	ShaderProgramVariable(loc_, name_, glDataType_, UNIFORM, fatherSProg_)
{}


#endif
