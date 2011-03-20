#ifndef MTL_USER_DEFINED_VAR_RUNTIME_H
#define MTL_USER_DEFINED_VAR_RUNTIME_H

#include "MtlUserDefinedVar.h"


/// This holds a copy of the MtlUserDefinedVar's data in order to be changed inside the main loop
class MtlUserDefinedVarRuntime
{
	friend class ConstructVisitor;

	public:
		/// The data union. The Texture resource is read-only at runtime
		typedef boost::variant<float, Vec2, Vec3, Vec4, const RsrcPtr<Texture>*, MtlUserDefinedVar::Fai> DataVariant;

		/// The one and only constructor
		MtlUserDefinedVarRuntime(const MtlUserDefinedVar& rsrc);

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
		/// @}

	private:
		/// Initialize the data using a visitor
		class ConstructVisitor: public boost::static_visitor<void>
		{
			public:
				MtlUserDefinedVarRuntime& udvr;

				ConstructVisitor(MtlUserDefinedVarRuntime& udvr_): udvr(udvr_) {}

				void operator()(float x) const;
				void operator()(const Vec2& x) const;
				void operator()(const Vec3& x) const;
				void operator()(const Vec4& x) const;
				void operator()(const RsrcPtr<Texture>& x) const;
				void operator()(MtlUserDefinedVar::Fai x) const;
		};

		DataVariant data;
		const MtlUserDefinedVar& rsrc; ///< Know the resource
};


#endif
