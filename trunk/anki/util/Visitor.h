#ifndef ANKI_UTIL_VISITOR_H
#define ANKI_UTIL_VISITOR_H


namespace anki {


//==============================================================================

// Forward declaration
template<typename... Types>
struct ConstVisitor;


// Specialized for one and many
template<typename First, typename... Types>
struct ConstVisitor<First, Types...>: ConstVisitor<Types...>
{
	using ConstVisitor<Types...>::visit;
	virtual void visit(const First&) = 0;
};


// Specialized for one
template<typename First>
struct ConstVisitor<First>
{
	virtual void visit(const First&) = 0;
};


//==============================================================================

// Forward declaration
template<typename... Types>
struct MutableVisitor;


// Specialized for one and many
template<typename First, typename... Types>
struct MutableVisitor<First, Types...>: MutableVisitor<Types...>
{
	using MutableVisitor<Types...>::visit;
	virtual void visit(First&) = 0;
};


// Specialized for one
template<typename First>
struct MutableVisitor<First>
{
	virtual void visit(First&) = 0;
};


//==============================================================================

/// Visitor for getting the type id
template<typename... Types>
struct GetTypeIdVisitor
{
	// Forward
	template<typename... Types_>
	struct Helper;

	// Specialized for one and many
	template<typename First, typename... Types_>
	struct Helper<First, Types_...>: Helper<Types_...>
	{
		using Helper<Types_...>::visit;

		void visit(const First& x)
		{
			Helper<Types_...>::id = sizeof...(Types_);
		}
	};

	// Specialized for one
	template<typename First>
	struct Helper<First>: ConstVisitor<Types...>
	{
		int id;

		void visit(const First&)
		{
			id = 0;
		}
	};

	typedef Helper<Types...> Type;
};


//==============================================================================

/// XXX
template<typename... Types>
struct DummyVisitor
{
	// Forward
	template<typename... Types_>
	struct Helper;

	// Declare
	template<typename First, typename... Types_>
	struct Helper<First, Types_...>: Helper<Types_...>
	{
		void visit(const First&)
		{}
	};

	// Specialized for one
	template<typename First>
	struct Helper<First>: ConstVisitor<Types...>
	{
		void visit(const First&)
		{}
	};

	typedef Helper<Types...> Type;
};


//==============================================================================

/// Visitable class
template<typename... Types>
struct Visitable
{
	typedef MutableVisitor<Types...> MutableVisitorType;
	typedef ConstVisitor<Types...> ConstVisitorType;
	typedef typename GetTypeIdVisitor<Types...>::Type GetTypeIdVisitorType;
	typedef typename DummyVisitor<Types...>::Type DummyVisitorType;

	/// Visitor accept
	virtual void accept(MutableVisitorType& v) = 0;
	/// Visitor accept
	virtual void accept(ConstVisitorType& v) const = 0;
};


} // end namespace


#endif
