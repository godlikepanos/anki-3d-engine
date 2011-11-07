#ifndef ANKI_RESOURCE_MATERIAL_USER_VARIABLE_H
#define ANKI_RESOURCE_MATERIAL_USER_VARIABLE_H

#include "anki/resource/MaterialVariable.h"
#include "anki/math/Math.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramUniformVariable.h"
#include <boost/variant.hpp>


namespace anki {


class Texture;


/// XXX
class MaterialUserVariable: public MaterialVariable
{
	public:
		/// The data union
		typedef boost::variant<float, Vec2, Vec3, Vec4, TextureResourcePointer>
			DataVariant;

		/// @name Constructors & destructor
		/// @{
		MaterialUserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			float val);

		MaterialUserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec2& val);

		MaterialUserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec3& val);

		MaterialUserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec4& val);

		MaterialUserVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const char* texFilename);

		~MaterialUserVariable();
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
		const ShaderProgramUniformVariable&
			getShaderProgramUniformVariable(PassType p) const;
		/// @}

	private:
		DataVariant data;
};


inline const ShaderProgramUniformVariable&
	MaterialUserVariable::getShaderProgramUniformVariable(PassType p) const
{
	return static_cast<const ShaderProgramUniformVariable&>(
		getShaderProgramVariable(p));
}


} // end namespace


#endif
