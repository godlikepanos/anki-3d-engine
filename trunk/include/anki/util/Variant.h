#ifndef ANKI_UTIL_VARIANT_H
#define ANKI_UTIL_VARIANT_H

#include "anki/util/Visitor.h"
#include "anki/util/Exception.h"
#include <iosfwd>
#include <memory>

namespace anki {

/// @addtogroup util_patterns
/// @{

// Forwards
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
	typedef Visitable<VariantDerived<Types, VariantBase<Types...>>...> Base;
	typedef VariantBase<Types...> Self;
	typedef typename BaseType::ConstVisitor ConstVisitor;
	typedef typename BaseType::MutableVisitor MutableVisitor;

	static const int TEMPLATE_PARAMETERS_NUM = sizeof...(Types);

	/// Default constructor
	VariantBase(int typeId_)
		: typeId(typeId_)
	{}

	virtual ~VariantBase()
	{}

	uint getTypeId() const
	{
		return typeId;
	}

	/// Get the parameter's value. It throws if the @a Data is incompatible
	template<typename Data>
	const Data& get() const
	{
		checkValue<Data>();
		return static_cast<const VariantDerived<Data, SelfType>&>(*this).get();
	}

	/// Generic setter. It throws if the @a Data is incompatible
	template<typename Data>
	void set(const Data& x)
	{
		checkValue<Data>();
		static_cast<VariantDerived<Data, SelfType>&>(*this).set(x);
	}

	/// Compare the derived classes
	virtual bool equal(const SelfType& b) const = 0;

	/// Copy the derived classes
	virtual void copy(const SelfType& b) = 0;

	/// Check if two derived are the same type
	bool areSameType(const SelfType& b) const
	{
		return typeId == b.typeId;
	}

	/// Print
	friend std::ostream& operator<<(std::ostream& s, const SelfType& x)
	{
		PrintVisitor v(s);
		x.accept(v);
		return s;
	}

protected:
	static constexpr char* INCOMPATIBLES_ERR = "Incompatible parameter types";

private:
	template<typename... Types_>
	struct PrintVisitor;

	template<typename First, typename... Types_>
	struct PrintVisitor<First, Types_...>: PrintVisitor<Types_...>
	{
		PrintVisitor(std::ostream& stream)
			: PrintVisitor<Types_...>(stream)
		{}

		void visit(const First& x)
		{
			PrintVisitor<Types_...>::stream << x;
		}
	};

	template<typename First>
	struct PrintVisitor<First>: ConstVisitorType
	{
		std::ostream& stream;

		PrintVisitor(std::ostream& stream_)
			: stream(stream_)
		{}

		void visit(const First& x)
		{
			stream << x;
		}
	};

	using PrintVisitor = PrintVisitor<VariantDerived<Types, SelfType>...>;

	uint typeId; ///< type ID

	/// Use const struff to check if @a Data is compatible with this
	template<typename Data>
	void checkValue() const
	{
		typedef VariantDerived<Data, SelfType> Vd;
		const int id = BaseType::template getTypeId<Vd>();
		if(id != typeId)
		{
			throw ANKI_EXCEPTION(INCOMPATIBLES_ERR);
		}
	}
};

/// VariantDerived template class. Using the @a TData and the @a TVariantBase 
/// it constructs a variant class
template<typename TData, typename TVariantBase>
class VariantDerived: public TVariantBase
{
public:
	typedef TData Data;
	typedef TVariantBase Base;
	typedef VariantDerived<DataType, BaseType> Self;
	typedef typename BaseType::ConstVisitor ConstVisitor;
	typedef typename BaseType::MutableVisitor MutableVisitor;

	static const int TYPE_ID =
		BaseType::BaseType::template getTypeId<SelfType>();

	/// Constructor
	VariantDerived()
		: BaseType(TYPE_ID), data(DataType())
	{}

	/// Constructor
	VariantDerived(const DataType& x)
		: BaseType(TYPE_ID), data(x)
	{}

	/// Copy constructor
	VariantDerived(const SelfType& b)
		: BaseType(TYPE_ID), data(b.data)
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

	/// Implements the VariantBase::equal
	bool equal(const BaseType& b) const
	{
		if(!BaseType::areSameType(b))
		{
			throw ANKI_EXCEPTION(BaseType::INCOMPATIBLES_ERR);
		}

		return *this == static_cast<const SelfType&>(b);
	}

	/// Implements the VariantBase::copy
	void copy(const BaseType& b)
	{
		if(!BaseType::areSameType(b))
		{
			throw ANKI_EXCEPTION(BaseType::INCOMPATIBLES_ERR);
		}

		data = static_cast<const SelfType&>(b).data;
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

	/// Default constructor
	Variant()
	{}

	/// Constructor with data
	template<typename Data>
	explicit Variant(const Data& x)
		: ptr(new VariantDerived<Data, VariantBaseType>(x))
	{}

	/// Copy constructor
	Variant(const SelfType& x)
	{
		CopyVisitorType v(*this);
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

	/// Copy
	SelfType& operator=(const SelfType& b)
	{
		ptr->copy(*b.ptr);
		return *this;
	}

	/// Compare
	bool operator==(const SelfType& b) const
	{
		return ptr->equal(*b.ptr);
	}

	/// set value
	template<typename Data>
	SelfType& operator=(const Data& b)
	{
		set<Data>(b);
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& s, const SelfType& x)
	{
		s << *x.ptr;
		return s;
	}

private:
	// Forward
	template<typename... Types_>
	struct CopyVisitor;

	/// Visitor used to copy the data of a VariantBase to a Variant
	template<typename First, typename... Types_>
	struct CopyVisitor<First, Types_...>: CopyVisitor<Types_...>
	{
		using CopyVisitor<Types_...>::visit;

		CopyVisitor(SelfType& a)
			: CopyVisitor<Types_...>(a)
		{}

		void visit(const First& b)
		{
			CopyVisitor<Types_...>::a.ptr.reset(new First(b.get()));
		}
	};

	// Specialized for one
	template<typename First>
	struct CopyVisitor<First>: VariantBaseType::ConstVisitorType
	{
		SelfType& a;

		CopyVisitor(SelfType& a_)
			: a(a_)
		{}

		void visit(const First& b)
		{
			a.ptr.reset(new First(b.get()));
		}
	};

	typedef CopyVisitor<VariantDerived<Types, VariantBaseType>...>
		CopyVisitorType;

	/// The only data member, a pointer to a VariantBase class
	std::unique_ptr<VariantBaseType> ptr;
};
/// @}

} // end namespace anki

#endif
