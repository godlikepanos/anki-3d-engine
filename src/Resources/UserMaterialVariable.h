#ifndef USER_MATERIAL_VARIABLE_H
#define USER_MATERIAL_VARIABLE_H

#include "MaterialVariable.h"
#include "Math/Math.h"
#include "RsrcPtr.h"
#include <boost/variant.hpp>


class Texture;
class UniformShaderProgramVariable;


/// XXX
class UserMaterialVariable: public MaterialVariable
{
	public:
		/// The data union
		typedef boost::variant<float, Vec2, Vec3, Vec4, RsrcPtr<Texture> >
			DataVariant;

		/// @name Constructors & destructor
		/// @{
		UserMaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			float val);

		UserMaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec2& val);

		UserMaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec3& val);

		UserMaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const Vec4& val);

		UserMaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr,
			const char* texFilename);

		~UserMaterialVariable();
		/// @}

		/// @name Accessors
		/// @{
		const DataVariant& getDataVariant() const {return data;}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data
		/// type
		template<typename Type>
		const Type& get() const {return boost::get<Type>(data);}

		const UniformShaderProgramVariable&
			getColorPassUniformShaderProgramVariable() const;

		const UniformShaderProgramVariable&
			getDepthPassUniformShaderProgramVariable() const;
		/// @}

	private:
		DataVariant data;
};


#endif
