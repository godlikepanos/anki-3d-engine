#ifndef SHADER_PROGRAM_UNIFORM_VARIABLE_H
#define SHADER_PROGRAM_UNIFORM_VARIABLE_H

#include "Variable.h"
#include "Math/Math.h"


class Texture;


namespace shader_program {


/// Uniform shader variable
class UniformVariable: public Variable
{
	public:
		UniformVariable(int loc, const char* name,
			GLenum glDataType,
			const ShaderProgram& fatherSProg);

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

	private:
		/// Standard set uniform checks
		/// - Check if initialized
		/// - if the current shader program is the var's shader program
		/// - if the GL driver gives the same location as the one the var has
		void doSanityChecks() const;
};


inline UniformVariable::UniformVariable(
	int loc_, const char* name_,
	GLenum glDataType_, const ShaderProgram& fatherSHADER_PROGRAM_)
:	Variable(loc_, name_, glDataType_, UNIFORM, fatherSHADER_PROGRAM_)
{}


} // end namespace


#endif
