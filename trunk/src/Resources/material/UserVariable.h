#ifndef MATERIAL_USER_VARIABLE_H
#define MATERIAL_USER_VARIABLE_H

#include "Variable.h"
#include "Math/Math.h"
#include "../RsrcPtr.h"
#include "../shader_program/UniformVariable.h"
#include <boost/variant.hpp>


class Texture;


namespace material {


/// XXX
class UserVariable: public Variable
{
	public:
		/// The data union
		typedef boost::variant<float, Vec2, Vec3, Vec4, RsrcPtr<Texture> >
			DataVariant;

		/// @name Constructors & destructor
		/// @{
		UserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			float val);

		UserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec2& val);

		UserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec3& val);

		UserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec4& val);

		UserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const char* texFilename);

		~UserVariable();
		/// @}

		/// @name Accessors
		/// @{
		const DataVariant& getDataVariant() const {return data;}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data
		/// type
		template<typename Type>
		const Type& getValue() const {return boost::get<Type>(data);}

		/// Uses static cast to get the uniform
		const shader_program::UniformVariable&
			getShaderProgramUniformVariable(PassType p) const;
		/// @}

	private:
		DataVariant data;
};


inline const shader_program::UniformVariable&
	UserVariable::getShaderProgramUniformVariable(PassType p) const
{
	return static_cast<const shader_program::UniformVariable&>(
		getShaderProgramVariable(p));
}


} // end namespace


#endif
