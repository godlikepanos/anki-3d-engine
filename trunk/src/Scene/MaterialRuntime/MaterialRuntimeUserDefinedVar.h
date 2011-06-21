#ifndef MATERIAL_RUNTIME_USER_DEFINED_VAR_H
#define MATERIAL_RUNTIME_USER_DEFINED_VAR_H

#include "MtlUserDefinedVar.h"


/// This holds a copy of the MtlUserDefinedVar's data in order to be changed inside the main loop
class MaterialRuntimeUserDefinedVar
{
	friend class ConstructVisitor;

	public:
		/// The data union. The Texture resource is read-only at runtime
		typedef boost::variant<float, Vec2, Vec3, Vec4, const Texture*, MtlUserDefinedVar::Fai> DataVariant;

		/// The one and only constructor
		MaterialRuntimeUserDefinedVar(const MtlUserDefinedVar& rsrc);

		/// @name Accessors
		/// @{
		const SProgUniVar& getUniVar() const {return rsrc.getUniVar();}

		const std::string& getName() const {return getUniVar().getName();}

		GETTER_RW(DataVariant, data, getDataVariant)

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data type
		template<typename Type>
		const Type& get() const {return boost::get<Type>(data);}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data type
		template<typename Type>
		Type& get() {return boost::get<Type>(data);}

		template<typename Type>
		void set(const Type& v) {boost::get<Type>(data) = v;}
		/// @}

	private:
		/// Initialize the data using a visitor
		class ConstructVisitor: public boost::static_visitor<void>
		{
			public:
				MaterialRuntimeUserDefinedVar& udvr;

				ConstructVisitor(MaterialRuntimeUserDefinedVar& udvr_): udvr(udvr_) {}

				/// Template method that applies to all DataVariant values except texture resource
				template<typename Type>
				void operator()(const Type& x) const {udvr.data = x;}
		};

		DataVariant data;
		const MtlUserDefinedVar& rsrc; ///< Know the resource
};


#endif
