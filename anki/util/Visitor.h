// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// The file contains visitor concepts
///
/// @par The main classes are:
/// @li Visitable: For non related types
/// @li VisitableCommonBase: For types with common base

#pragma once

#include <anki/util/Assert.h>
#include <anki/util/Array.h>
#include <anki/util/StdTypes.h>

namespace anki
{

/// @addtogroup util_patterns
/// @{

/// Namespace for visitor internal classes
namespace visitor_detail
{

/// A smart struct that given a type and a list of types finds a const integer indicating the type's position from the
/// back of the list.
///
/// Example of usage:
/// @code
/// GetVariadicTypeId<int, float, std::string>::get<float>() == 1
/// GetVariadicTypeId<int, float, std::string>::get<int>() == 0
/// GetVariadicTypeId<int, float, std::string>::get<char>() // Compiler error
/// @endcode
template<typename... Types>
struct GetVariadicTypeId
{
	// Forward
	template<typename Type, typename... Types_>
	struct Helper;

	// Declaration
	template<typename Type, typename TFirst, typename... Types_>
	struct Helper<Type, TFirst, Types_...> : Helper<Type, Types_...>
	{
	};

	// Specialized
	template<typename Type, typename... Types_>
	struct Helper<Type, Type, Types_...>
	{
		static const I ID = sizeof...(Types_);
	};

	/// Get the id
	template<typename Type>
	static constexpr I get()
	{
		return sizeof...(Types) - Helper<Type, Types...>::ID - 1;
	}
};

/// A struct that from a variadic arguments list it returnes the type using an ID.
/// Example of usage:
/// @code
/// GetTypeUsingId<std::string, int, float>::DataType<0> x = "a string";
/// GetTypeUsingId<std::string, int, float>::DataType<1> y = 123;
/// GetTypeUsingId<std::string, int, float>::DataType<2> y = 123.456f;
/// @endcode
template<typename... Types>
struct GetTypeUsingId
{
	// Forward declaration
	template<I id, typename... Types_>
	struct Helper;

	// Declaration
	template<I id, typename TFirst, typename... Types_>
	struct Helper<id, TFirst, Types_...> : Helper<id - 1, Types_...>
	{
	};

	// Specialized
	template<typename TFirst, typename... Types_>
	struct Helper<0, TFirst, Types_...>
	{
		using DataType = TFirst;
	};

	template<I id>
	using DataType = typename Helper<id, Types...>::DataType;
};

} // end namespace visitor_detail

/// Visitable class
template<typename... Types>
class Visitable
{
public:
	Visitable()
	{
	}

	template<typename T>
	Visitable(T* t)
	{
		setupVisitable(t);
	}

	I getVisitableTypeId() const
	{
		return m_what;
	}

	template<typename T>
	static constexpr I getVariadicTypeId()
	{
		return visitor_detail::GetVariadicTypeId<Types...>::template get<T>();
	}

	/// Apply visitor
	template<typename TVisitor>
	void acceptVisitor(TVisitor& v)
	{
		ANKI_ASSERT(m_what != -1 && address != nullptr);
		acceptVisitorInternal<TVisitor, Types...>(v);
	}

	/// Apply visitor (const version)
	template<typename TVisitor>
	void acceptVisitor(TVisitor& v) const
	{
		ANKI_ASSERT(m_what != -1 && address != nullptr);
		acceptVisitorInternalConst<TVisitor, Types...>(v);
	}

	/// Setup the data
	template<typename T>
	void setupVisitable(T* t)
	{
		// Null arg. Setting for second time? Now allowed
		ANKI_ASSERT(t != nullptr);
		ANKI_ASSERT(address == nullptr && m_what == -1);
		address = t;
		m_what = visitor_detail::GetVariadicTypeId<Types...>::template get<T>();
	}

private:
	I m_what = -1; ///< The type ID
	void* address = nullptr; ///< The address to the data

	/// @name Accept visitor template methods
	/// @{
	template<typename TVisitor, typename TFirst>
	void acceptVisitorInternal(TVisitor& v)
	{
		switch(m_what)
		{
		case 0:
			v.template visit(*reinterpret_cast<TFirst*>(address));
			break;
		default:
			ANKI_ASSERT(0 && "Wrong type ID");
			break;
		}
	}

	template<typename TVisitor, typename TFirst, typename TSecond, typename... Types_>
	void acceptVisitorInternal(TVisitor& v)
	{
		constexpr I i = sizeof...(Types) - sizeof...(Types_) - 1;

		switch(m_what)
		{
		case i:
			v.template visit(*reinterpret_cast<TSecond*>(address));
			break;
		default:
			acceptVisitorInternal<TVisitor, TFirst, Types_...>(v);
			break;
		}
	}

	template<typename TVisitor, typename TFirst>
	void acceptVisitorInternalConst(TVisitor& v) const
	{
		switch(m_what)
		{
		case 0:
			v.template visit(*reinterpret_cast<const TFirst*>(address));
			break;
		default:
			ANKI_ASSERT(0 && "Wrong type ID");
			break;
		}
	}

	template<typename TVisitor, typename TFirst, typename TSecond, typename... Types_>
	void acceptVisitorInternalConst(TVisitor& v) const
	{
		constexpr I i = sizeof...(Types) - sizeof...(Types_) - 1;

		switch(m_what)
		{
		case i:
			v.template visit(*reinterpret_cast<const TSecond*>(address));
			break;
		default:
			acceptVisitorInternalConst<TVisitor, TFirst, Types_...>(v);
			break;
		}
	}
	/// @}
};

/// Visitable for types with common base
/// @tparam TBase The base class
/// @tparam Types The types that compose the visitable. They are derived from TBase
///
/// @note In debug mode it uses dynamic cast as an extra protection reason
template<typename TBase, typename... Types>
class VisitableCommonBase
{
public:
#if ANKI_EXTRA_CHECKS
	// Allow dynamic cast in acceptVisitor
	virtual ~VisitableCommonBase()
	{
	}
#endif

	I getVisitableTypeId() const
	{
		ANKI_ASSERT(m_what != -1);
		return m_what;
	}

	template<typename T>
	static constexpr I getVariadicTypeId()
	{
		return visitor_detail::GetVariadicTypeId<Types...>::template get<T>();
	}

	/// Check if this is of type
	template<typename T>
	Bool isTypeOf() const
	{
		return getVariadicTypeId<T>() == getVisitableTypeId();
	}

	/// Apply mutable visitor
	template<typename TVisitor>
	ANKI_USE_RESULT Error acceptVisitor(TVisitor& v)
	{
		ANKI_ASSERT(m_what != -1);
		return acceptVisitorInternal<TVisitor, Types...>(v);
	}

	/// Apply const visitor
	template<typename TVisitor>
	ANKI_USE_RESULT Error acceptVisitor(TVisitor& v) const
	{
		ANKI_ASSERT(m_what != -1);
		return acceptVisitorInternalConst<TVisitor, Types...>(v);
	}

	/// Setup the type ID
	template<typename T>
	void setupVisitable(const T*)
	{
		ANKI_ASSERT(m_what == -1); // Setting for second time not allowed
		m_what = visitor_detail::GetVariadicTypeId<Types...>::template get<T>();
	}

private:
	I m_what = -1; ///< The type ID

	/// @name Accept visitor template methods
	/// @{
	template<typename TVisitor, typename TFirst>
	ANKI_USE_RESULT Error acceptVisitorInternal(TVisitor& v)
	{
		Error err = Error::NONE;

		switch(m_what)
		{
		case 0:
		{
#if ANKI_EXTRA_CHECKS
			TFirst* base = dynamic_cast<TFirst*>(this);
			ANKI_ASSERT(base != nullptr);
#else
			TFirst* base = static_cast<TFirst*>(this);
#endif
			err = v.template visit(*base);
		}
		break;
		default:
			ANKI_ASSERT(0 && "Wrong type ID");
			break;
		}

		return err;
	}

	template<typename TVisitor, typename TFirst, typename TSecond, typename... Types_>
	ANKI_USE_RESULT Error acceptVisitorInternal(TVisitor& v)
	{
		Error err = Error::NONE;
		constexpr I i = sizeof...(Types) - sizeof...(Types_) - 1;

		switch(m_what)
		{
		case i:
		{
#if ANKI_EXTRA_CHECKS
			TSecond* base = dynamic_cast<TSecond*>(this);
			ANKI_ASSERT(base != nullptr);
#else
			TSecond* base = static_cast<TSecond*>(this);
#endif
			err = v.template visit(*base);
		}
		break;
		default:
			err = acceptVisitorInternal<TVisitor, TFirst, Types_...>(v);
			break;
		}

		return err;
	}

	template<typename TVisitor, typename TFirst>
	ANKI_USE_RESULT Error acceptVisitorInternalConst(TVisitor& v) const
	{
		Error err = Error::NONE;

		switch(m_what)
		{
		case 0:
		{
#if ANKI_EXTRA_CHECKS
			const TFirst* base = dynamic_cast<const TFirst*>(this);
			ANKI_ASSERT(base != nullptr);
#else
			const TFirst* base = static_cast<const TFirst*>(this);
#endif
			err = v.template visit(*base);
			break;
		}
		default:
			ANKI_ASSERT(0 && "Wrong type ID");
			break;
		}

		return err;
	}

	template<typename TVisitor, typename TFirst, typename TSecond, typename... Types_>
	ANKI_USE_RESULT Error acceptVisitorInternalConst(TVisitor& v) const
	{
		Error err = Error::NONE;
		constexpr I i = sizeof...(Types) - sizeof...(Types_) - 1;

		switch(m_what)
		{
		case i:
		{
#if ANKI_EXTRA_CHECKS
			const TSecond* base = dynamic_cast<const TSecond*>(this);
			ANKI_ASSERT(base != nullptr);
#else
			const TSecond* base = static_cast<const TSecond*>(this);
#endif
			err = v.template visit(*base);
			break;
		}
		default:
			err = acceptVisitorInternalConst<TVisitor, TFirst, Types_...>(v);
			break;
		}

		return err;
	}
	/// @}
};
/// @}

} // end namespace anki
