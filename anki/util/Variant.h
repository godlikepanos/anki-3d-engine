#ifndef ANKI_UTIL_VARIANT_H
#define ANKI_UTIL_VARIANT_H

#include "anki/util/Visitor.h"
#include "anki/util/Exception.h"
#include <iosfwd>
#include <memory>


namespace anki {


// Forward declaration of VariantDerived
template<typename Data, typename VariantBase>
class VariantDerived;

template<typename... Types>
class VariantBase;


/// The variant base class
template<typename... Types>
class VariantBase:
	public Visitable<VariantDerived<Types, VariantBase<Types...>>...>
{
public:
	typedef Visitable<VariantDerived<Types, VariantBase<Types...>>...> BaseType;
	typedef VariantBase<Types...> SelfType;
	typedef typename BaseType::ConstVisitorType ConstVisitorType;
	typedef typename BaseType::MutableVisitorType MutableVisitorType;
	typedef typename BaseType::GetTypeIdVisitorType GetTypeIdVisitorType;

	static const int TEMPLATE_PARAMETERS_NUM = sizeof...(Types);

	/// Default constructor
	VariantBase()
		: typeId(-1)
	{}

	virtual ~VariantBase()
	{}

	uint getTypeId() const
	{
		return typeId;
	}

	/// Get the parameter's value. It throws if the @a T is incompatible
	template<typename T>
	const T& get() const
	{
		checkValue<T>();
		return static_cast<const VariantDerived<T, SelfType>&>(*this).get();
	}

	/// Generic setter. It throws if the @a T is incompatible
	template<typename T>
	void set(const T& x)
	{
		checkValue<T>();
		static_cast<VariantDerived<T, SelfType>&>(*this).set(x);
	}

	/// Compare the derived classes
	virtual bool operator==(const SelfType& b) const = 0;

	/// Check if two derived are the same type
	bool areSameType(const SelfType& b) const
	{
		return typeId == b.typeId;
	}

protected:
	/// Get the id the hard way (using a visitor). Don't call it from
	/// constructors
	void setIdHard()
	{
		GetTypeIdVisitorType v;
		accept(v);
		typeId = v.id;
	}

	static constexpr char* INCOMPATIBLES_ERR = "Incompatible parameter types";

private:
	/// Used to check the compatibility of types
	template<typename TData>
	struct CheckDataTypeVisitor:
		Visitable<VariantDerived<Types, SelfType>...>::DummyVisitorType
	{
		using Visitable<VariantDerived<Types, SelfType>...>::
			DummyVisitorType::visit;

		bool b;

		CheckDataTypeVisitor()
			: b(false)
		{}

		void visit(const VariantDerived<TData, SelfType>&)
		{
			b = true;
		}
	};

	uint typeId; ///< type ID

	///_Use a visitor provided from a derived class to check for compatible
	/// types
	template<typename T>
	void checkValue() const
	{
		CheckDataTypeVisitor<T> v;
		accept(v);
		if(!v.b)
		{
			throw ANKI_EXCEPTION(INCOMPATIBLES_ERR);
		}
	}
};


/// VariantDerived template class
///
/// Using the @a TData and the @a TVariantBase it constructs a variant class
template<typename TData, typename TVariantBase>
class VariantDerived: public TVariantBase
{
public:
	typedef TData DataType;
	typedef TVariantBase BaseType;
	typedef VariantDerived<DataType, BaseType> SelfType;
	typedef typename BaseType::ConstVisitorType ConstVisitorType;
	typedef typename BaseType::MutableVisitorType MutableVisitorType;

	/// Constructor
	VariantDerived()
		: data(DataType())
	{
		BaseType::setIdHard();
	}

	/// Constructor
	VariantDerived(const DataType& x)
		: data(x)
	{
		BaseType::setIdHard();
	}

	/// Copy constructor
	VariantDerived(const SelfType& b)
		: data(b.data)
	{}

	/// @name Accessors
	/// @{
	void set(const DataType& x)
	{
		data = x;
	}
	const DataType& get() const
	{
		return data;
	}
	/// @}

	/// Compare
	bool operator==(const SelfType& b) const
	{
		return data == b.data;
	}

	/// Implements the VariantBase::operator==
	bool operator==(const BaseType& b) const
	{
		if(!areSameType(b))
		{
			throw ANKI_EXCEPTION(BaseType::INCOMPATIBLES_ERR);
		}

		return *this == static_cast<const SelfType&>(b);
	}

	/// Implements VariantBase::accept
	void accept(ConstVisitorType& v) const
	{
		v.visit(*this);
	}
	/// Implements VariantBase::accept
	void accept(MutableVisitorType& v)
	{
		v.visit(*this);
	}

	/// Print
	friend std::ostream& operator<<(std::ostream& s, const SelfType& x)
	{
		s << x.data;
		return s;
	}

private:
	DataType data; ///< The data
};


/// XXX
/// @note It is no possible to have default constructor because we cannot have
/// template default constructor
template<typename... Types>
class Variant
{
public:
	typedef Variant<Types...> SelfType;
	typedef VariantBase<Types...> VariantBaseType;

	/// Constructor with data
	template<typename Data>
	Variant(const Data& x)
		: ptr(new VariantDerived<Data, VariantBaseType>(x))
	{}

	/// Copy constructor
	Variant(const SelfType& x)
	{
		YYY v(*this);
		x.ptr->accept(v);
	}

	~Variant()
	{}

	/// Get the parameter's value. It throws if the @a Data is incompatible
	template<typename Data>
	const Data& get() const
	{
		return ptr->get<Data>();
	}

	/// Generic setter. It throws if the @a Data is incompatible
	template<typename Data>
	void set(const Data& x)
	{
		ptr->set<Data>(x);
	}

private:
	template<typename... Types_>
	struct XXX;

	template<typename First, typename... Types_>
	struct XXX<First, Types_...>: XXX<Types_...>
	{
		using XXX<Types_...>::visit;

		XXX(SelfType& a)
			: XXX<Types_...>(a)
		{}

		void visit(const First& b)
		{
			XXX<Types_...>::a.ptr.reset(new First(b.get()));
		}
	};

	template<typename First>
	struct XXX<First>: VariantBaseType::ConstVisitorType
	{
		SelfType& a;

		XXX(SelfType& a_)
			: a(a_)
		{}

		void visit(const First& b)
		{
			a.ptr.reset(new First(b.get()));
		}
	};

	typedef XXX<VariantDerived<Types, VariantBaseType>...> YYY;

	std::unique_ptr<VariantBaseType> ptr;
};


} // end namespace


#endif
