#ifndef ANKI_SCENE_MATERIAL_RUNTIME_VARIABLE_H
#define ANKI_SCENE_MATERIAL_RUNTIME_VARIABLE_H

#include "anki/math/Math.h"
#include "anki/resource/Resource.h"
#include <boost/variant.hpp>


namespace anki {


class Texture;
class MaterialVariable;


/// Variable of runtime materials
/// This holds a copy of the MtlUserDefinedVar's data in order to be changed
/// inside the main loop
class MaterialRuntimeVariable
{
	public:
		typedef const TextureResourcePointer* ConstPtrRsrcPtrTexture;

		/// The data union. The Texture resource is read-only at runtime
		/// Don't EVER replace the texture with const Texture*. The asynchronous
		/// operations will fail
		typedef boost::variant<float, Vec2, Vec3, Vec4, Mat3,
			Mat4, ConstPtrRsrcPtrTexture> Variant;

		/// Constructor
		MaterialRuntimeVariable(const MaterialVariable& mv);

		/// Destructor
		~MaterialRuntimeVariable();

		/// @name Accessors
		/// @{
		const MaterialVariable& getMaterialVariable() const
		{
			return mvar;
		}

		const Variant& getDataVariant() const
		{
			return data;
		}
		Variant& getDataVariant()
		{
			return data;
		}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data
		/// type
		template<typename Type>
		const Type& getValue() const
		{
			return boost::get<Type>(data);
		}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data
		/// type
		template<typename Type>
		Type& getValue()
		{
			return boost::get<Type>(data);
		}

		template<typename Type>
		void setValue(const Type& v)
		{
			boost::get<Type>(data) = v;
		}

		int getBuildinId() const
		{
			return buildinId;
		}
		int& getBuildinId()
		{
			return buildinId;
		}
		void setBuildinId(const int x)
		{
			buildinId = x;
		}
		/// @}

	private:
		/// Initialize the data using a visitor
		class ConstructVisitor: public boost::static_visitor<void>
		{
			public:
				MaterialRuntimeVariable& var;

				ConstructVisitor(MaterialRuntimeVariable& var_)
				:	var(var_)
				{}

				/// Template method that applies to all DataVariant values
				/// except texture resource
				template<typename Type>
				void operator()(const Type& x) const
				{
					var.getDataVariant() = x;
				}
		};

		const MaterialVariable& mvar; ///< Know the resource
		Variant data; /// The data
		int buildinId;
};


// Declare specialized
template <>
void MaterialRuntimeVariable::ConstructVisitor::
	operator()<TextureResourcePointer >(const TextureResourcePointer& x) const;

// Declare specialized
template<>
MaterialRuntimeVariable::ConstPtrRsrcPtrTexture&
	MaterialRuntimeVariable::getValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>();

// Declare specialized
template<>
void MaterialRuntimeVariable::setValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>(
	const ConstPtrRsrcPtrTexture& v);


} // end namespace


#endif
