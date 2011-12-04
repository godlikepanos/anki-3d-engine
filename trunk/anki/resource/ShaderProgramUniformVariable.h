#ifndef ANKI_RESOURCE_SHADER_PROGRAM_UNIFORM_VARIABLE_H
#define ANKI_RESOURCE_SHADER_PROGRAM_UNIFORM_VARIABLE_H

#include "anki/resource/ShaderProgramVariable.h"
#include "anki/math/Math.h"
#include <vector>


namespace anki {


class Texture;


/// Uniform shader variable
class ShaderProgramUniformVariable: public ShaderProgramVariable
{
public:
	ShaderProgramUniformVariable(
		int loc,
		const char* name,
		GLenum glDataType,
		const ShaderProgram& fatherSProg)
		: ShaderProgramVariable(loc, name, glDataType, T_UNIFORM, fatherSProg)
	{}

	/// @name Set the var
	/// @{
	void set(const float x) const;
	void set(const Vec2& x) const;
	void set(const Vec3& x) const
	{
		set(&x, 1);
	}
	void set(const Vec4& x) const
	{
		set(&x, 1);
	}
	void set(const Mat3& x) const
	{
		set(&x, 1);
	}
	void set(const Mat4& x) const
	{
		set(&x, 1);
	}
	void set(const Texture& tex, uint texUnit) const;

	void set(const float x[], uint size) const;
	void set(const Vec2 x[], uint size) const;
	void set(const Vec3 x[], uint size) const;
	void set(const Vec4 x[], uint size) const;
	void set(const Mat3 x[], uint size) const;
	void set(const Mat4 x[], uint size) const;

	/// @tparam Type float, Vec2, etc
	template<typename Type>
	void set(const std::vector<Type>& c)
	{
		set(&c[0], c.size());
	}
	/// @}

private:
	/// Standard set uniform checks
	/// - Check if initialized
	/// - if the current shader program is the var's shader program
	/// - if the GL driver gives the same location as the one the var has
	void doSanityChecks() const;
};


} // end namespace


#endif
