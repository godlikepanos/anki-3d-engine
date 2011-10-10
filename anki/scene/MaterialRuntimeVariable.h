#ifndef MATERIAL_RUNTIME_VARIABLE_H
#define MATERIAL_RUNTIME_VARIABLE_H

#include "anki/math/Math.h"
#include "anki/resource/RsrcPtr.h"
#include <boost/variant.hpp>


class Texture;
class MaterialUserVariable;


/// Variable of runtime materials
/// This holds a copy of the MtlUserDefinedVar's data in order to be changed
/// inside the main loop
class MaterialRuntimeVariable
{
	public:
		typedef const RsrcPtr<Texture>* ConstPtrRsrcPtrTexture;

		/// The data union. The Texture resource is read-only at runtime
		/// Don't EVER replace the texture with const Texture*. The asynchronous
		/// operations will fail
		typedef boost::variant<float, Vec2, Vec3, Vec4,
			ConstPtrRsrcPtrTexture> DataVariant;

		/// Constructor
		MaterialRuntimeVariable(const MaterialUserVariable& umv);
		/// Destructor
		~MaterialRuntimeVariable();

		/// @name Accessors
		/// @{
		const MaterialUserVariable& getMaterialUserVariable() const
			{return umv;}


		const DataVariant& getDataVariant() const {return data;}
		DataVariant& getDataVariant() {return data;}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data
		/// type
		template<typename Type>
		const Type& getValue() const {return boost::get<Type>(data);}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data
		/// type
		template<typename Type>
		Type& getValue() {return boost::get<Type>(data);}

		template<typename Type>
		void setValue(const Type& v) {boost::get<Type>(data) = v;}
		/// @}

	private:
		/// Initialize the data using a visitor
		class ConstructVisitor: public boost::static_visitor<void>
		{
			public:
				MaterialRuntimeVariable& udvr;

				ConstructVisitor(MaterialRuntimeVariable& udmvr);

				/// Template method that applies to all DataVariant values
				/// except texture resource
				template<typename Type>
				void operator()(const Type& x) const
					{udvr.getDataVariant() = x;}
		};

		const MaterialUserVariable& umv; ///< Know the resource
		DataVariant data; /// The data
};


inline MaterialRuntimeVariable::ConstructVisitor::ConstructVisitor(
	MaterialRuntimeVariable& udvr_)
:	udvr(udvr_)
{}


#endif
