#ifndef MATERIAL_RUNTIME_VARIABLE_H
#define MATERIAL_RUNTIME_VARIABLE_H

#include "Util/Accessors.h"
#include "Math/Math.h"
#include "Resources/RsrcPtr.h"
#include "Resources/material/Fwd.h"
#include <boost/variant.hpp>


class Texture;


namespace material_runtime {


/// This holds a copy of the MtlUserDefinedVar's data in order to be changed
/// inside the main loop
class Variable
{
	public:
		typedef const RsrcPtr<Texture>* ConstPtrRsrcPtrTexture;

		/// The data union. The Texture resource is read-only at runtime
		/// Dont EVER replace the texture with const Texture*. The asynchronous
		/// operations will fail
		typedef boost::variant<float, Vec2, Vec3, Vec4,
			ConstPtrRsrcPtrTexture> DataVariant;

		/// Constructor
		Variable(const material::UserVariable& umv);
		/// Destructor
		~Variable();

		/// @name Accessors
		/// @{
		GETTER_R(material::UserVariable, umv, getUserVariable)
		GETTER_RW(DataVariant, data, getDataVariant)

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
				Variable& udvr;

				ConstructVisitor(Variable& udmvr);

				/// Template method that applies to all DataVariant values
				/// except texture resource
				template<typename Type>
				void operator()(const Type& x) const
					{udvr.getDataVariant() = x;}
		};

		const material::UserVariable& umv; ///< Know the resource
		DataVariant data; /// The data
};


inline Variable::ConstructVisitor::ConstructVisitor(
	material::UserVariable& udvr_)
:	udvr(udvr_)
{}


} // end namespace


#endif
