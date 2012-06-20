/// @file
/// The file contains visitor concepts

#ifndef ANKI_UTIL_VISITOR_H
#define ANKI_UTIL_VISITOR_H

#include "anki/util/Assert.h"
#include <array>

namespace anki {

/// @addtogroup util
/// @{

namespace visitor_detail {

/// A smart struct that given a type and a list of types finds a const integer
/// indicating the type's position from the back of the list.
///
/// Example of usage:
/// @code
/// GetVariadicTypeId<int, float, std::string>::get<float> == 1
/// GetVariadicTypeId<int, float, std::string>::get<int> == 0
/// GetVariadicTypeId<int, float, std::string>::get<char> // Compiler error
/// @endcode
template<typename... Types>
struct GetVariadicTypeId
{
	// Forward
	template<typename Type, typename... Types_>
	struct Helper;

	// Declaration
	template<typename Type, typename TFirst, typename... Types_>
	struct Helper<Type, TFirst, Types_...>: Helper<Type, Types_...>
	{};

	// Specialized
	template<typename Type, typename... Types_>
	struct Helper<Type, Type, Types_...>
	{
		static const int ID = sizeof...(Types_);
	};

	/// Get the id
	template<typename Type>
	static constexpr int get()
	{
		return sizeof...(Types) - Helper<Type, Types...>::ID - 1;
	}
};

/// A struct that from a variadic arguments list it returnes the type using an
/// ID.
/// Example of usage:
/// @code GetTypeUsingId<double, int, float>::DataType<0> @endcode
template<typename... Types>
struct GetTypeUsingId
{
	// Forward declaration
	template<int id, typename... Types_>
	struct Helper;

	// Declaration
	template<int id, typename TFirst, typename... Types_>
	struct Helper<id, TFirst, Types_...>: Helper<id - 1, Types_...>
	{};

	// Specialized
	template<typename TFirst, typename... Types_>
	struct Helper<0, TFirst, Types_...>
	{
		typedef TFirst DataType;
	};

	template<int id>
	using DataType = typename Helper<id, Types...>::DataType;
};

/// A simple struct that creates an array of pointers to functions that have
/// the same arguments but different body
template<typename TVisitor, typename... Types>
class JumpTable
{
public:
	using FuncPtr = void (*)(TVisitor&, void*);

	JumpTable()
	{
		init<Types...>();
	}

	/// Accessor
	FuncPtr operator[](int i) const
	{
		return jumps[i];
	}

private:
	/// Pointers to JumpPoint::visit static methods
	std::array<FuncPtr, sizeof...(Types)> jumps;

	template<typename T>
	static void visit(TVisitor& v, void* address)
	{
		v.template visit(*reinterpret_cast<T*>(address));
	}

	template<typename TFirst>
	void init()
	{
		jumps[0] = &visit<TFirst>;
	}

	template<typename TFirst, typename TSecond, typename... Types_>
	void init()
	{
		constexpr int i = sizeof...(Types) - sizeof...(Types_) - 1;
		jumps[i] = &visit<TSecond>;
		init<TFirst, Types_...>();
	}
};

/// A simple struct that contains a static field with jump points
template<typename TDerived, typename... Types>
struct VisitorWrapper
{
	static const JumpTable<TDerived, Types...> jumpTable;
};

template<typename TDerived, typename... Types>
const JumpTable<TDerived, Types...>
	VisitorWrapper<TDerived, Types...>::jumpTable;

} // end namespace visitor_detail

//==============================================================================
// Visitable                                                                   =
//==============================================================================

/// Visitable class
template<typename... Types>
class Visitable
{
public:
	Visitable()
	{}

	template<typename T>
	Visitable(T* t)
	{
		setupVisitable(t);
	}

	int getVisitableTypeId() const
	{
		return what;
	}

	template<typename T>
	static constexpr int getVariadicTypeId()
	{
		return visitor_detail::GetVariadicTypeId<Types...>::template get<T>();
	}

	/// Apply visitor
	template<typename TVisitor>
	void acceptVisitor(TVisitor& v)
	{
		ANKI_ASSERT(what != -1 && address != nullptr);
		visitor_detail::VisitorWrapper<TVisitor, Types...>::
			jumpTable[what](v, address);
	}

	/// Apply visitor (const version)
	template<typename TVisitor>
	void acceptVisitor(TVisitor& v) const
	{
		ANKI_ASSERT(what != -1 && address != nullptr);
		visitor_detail::VisitorWrapper<TVisitor, Types...>::
			jumpTable[what](v, address);
	}

	/// Setup the data
	template<typename T>
	void setupVisitable(T* t)
	{
		ANKI_ASSERT(t != nullptr); // Null arg
		// Setting for second time? Now allowed
		ANKI_ASSERT(address == nullptr && what == -1);
		address = t;
		what = visitor_detail::GetVariadicTypeId<Types...>::template get<T>();
	}

private:
	int what = -1; ///< The type ID
	void* address = nullptr; ///< The address to the data
};
/// @}

} // end namespace anki

#endif
