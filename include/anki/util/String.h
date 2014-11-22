// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_STRING_H
#define ANKI_UTIL_STRING_H

#include "anki/util/DArray.h"
#include "anki/util/Array.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/ScopeDestroyer.h"
#include <cstring>
#include <cmath> // For HUGE_VAL
#include <climits> // For LLONG_MAX
#include <cstdarg> // For var args

namespace anki {

// Forward
template<typename TAlloc>
class StringBase;

/// @addtogroup util_private
/// @{

namespace detail {

template<typename TNumber>
constexpr const char* toStringFormat()
{
	return nullptr;
}

#define ANKI_DEPLOY_TO_STRING(type_, string_) \
	template<> \
	constexpr const char* toStringFormat<type_>() \
	{ \
		return string_; \
	}

ANKI_DEPLOY_TO_STRING(I8, "%d")
ANKI_DEPLOY_TO_STRING(I16, "%d")
ANKI_DEPLOY_TO_STRING(I32, "%d")
ANKI_DEPLOY_TO_STRING(I64, "%lld")
ANKI_DEPLOY_TO_STRING(U8, "%u")
ANKI_DEPLOY_TO_STRING(U16, "%u")
ANKI_DEPLOY_TO_STRING(U32, "%u")
ANKI_DEPLOY_TO_STRING(U64, "%llu")
ANKI_DEPLOY_TO_STRING(F32, "%f")
ANKI_DEPLOY_TO_STRING(F64, "%f")

#undef ANKI_DEPLOY_TO_STRING

} // end namespace detail

/// @}

/// @addtogroup util_containers
/// @{

/// A wrapper on top of C strings. Used mainly for safety.
class CString
{
	template<typename TAlloc>
	friend class StringBase; // For the secret constructor

public:
	using Char = char;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	CString()  = default;

	CString(const Char* ptr) 
	:	m_ptr(ptr)
	{
		checkInit();
	}

	/// Copy constructor.
	CString(const CString& b) 
	:	m_ptr(b.m_ptr),
		m_length(b.m_length)
	{
		checkInit();
	}

	/// Copy.
	CString& operator=(const CString& b) 
	{
		m_ptr = b.m_ptr;
		m_length = b.m_length;
		return *this;
	}

	/// Return true if the string is initialized.
	operator Bool() const 
	{
		return !isEmpty();
	}

	/// Return char at the specified position.
	const Char& operator[](U pos) const 
	{
		checkInit();
		ANKI_ASSERT(pos <= getLength());
		return m_ptr[pos];
	}

	const Char* begin() const 
	{
		checkInit();
		return &m_ptr[0];
	}

	const Char* end() const 
	{
		checkInit();
		return &m_ptr[getLength()];
	}

	/// Return true if the string is not initialized.
	Bool isEmpty() const 
	{
		return m_ptr == nullptr || getLength() == 0;
	}

	Bool operator==(const CString& b) const 
	{
		if(m_ptr == nullptr || b.m_ptr == nullptr)
		{
			return m_ptr == b.m_ptr;
		}
		else
		{
			return std::strcmp(m_ptr, b.m_ptr) == 0;
		}
	}

	Bool operator!=(const CString& b) const 
	{
		return !((*this) == b);
	}

	Bool operator<(const CString& b) const 
	{
		if(m_ptr == nullptr || b.m_ptr == nullptr)
		{
			return false;
		}
		else
		{
			return std::strcmp(m_ptr, b.m_ptr) < 0;
		}
	}

	Bool operator<=(const CString& b) const 
	{
		if(m_ptr == nullptr || b.m_ptr == nullptr)
		{
			return m_ptr == b.m_ptr;
		}
		else
		{
			return std::strcmp(m_ptr, b.m_ptr) <= 0;
		}
	}

	Bool operator>(const CString& b) const 
	{
		if(m_ptr == nullptr || b.m_ptr == nullptr)
		{
			return false;
		}
		else
		{
			return std::strcmp(m_ptr, b.m_ptr) > 0;
		}
	}

	Bool operator>=(const CString& b) const 
	{
		if(m_ptr == nullptr || b.m_ptr == nullptr)
		{
			return m_ptr == b.m_ptr;
		}
		else
		{
			return std::strcmp(m_ptr, b.m_ptr) >= 0;
		}
	}

	/// Get the underlying C string.
	const char* get() const 
	{
		checkInit();
		return m_ptr;
	}

	/// Get the string length.
	U getLength() const 
	{
		if(m_length == 0 && m_ptr != nullptr)
		{
			m_length = std::strlen(m_ptr);
		}
		
		return m_length;
	}

	PtrSize find(const CString& cstr, PtrSize position = 0) const 
	{
		checkInit();
		ANKI_ASSERT(position < getLength());
		const Char* out = std::strstr(m_ptr, &cstr[0]);
		return (out == nullptr) ? NPOS : (out - m_ptr);
	}

	/// Convert to F64.
	ANKI_USE_RESULT Error toF64(F64& out) const
	{
		checkInit();
		Error err = ErrorCode::NONE;
		out = std::strtod(m_ptr, nullptr);

		if(out == HUGE_VAL)
		{
			ANKI_LOGE("Conversion failed");
			err = ErrorCode::USER_DATA;
		}

		return err;
	}

	/// Convert to F32.
	ANKI_USE_RESULT Error toF32(F32& out) const
	{
		F64 d;
		Error err = toF64(d);
		if(!err)
		{
			out = d;
		}

		return err;
	}

	/// Convert to I64.
	ANKI_USE_RESULT Error toI64(I64& out) const
	{
		checkInit();
		Error err = ErrorCode::NONE;
		out = std::strtoll(m_ptr, nullptr, 10);

		if(out == LLONG_MAX || out == LLONG_MIN)
		{
			ANKI_LOGE("Conversion failed");
			err = ErrorCode::USER_DATA;
		}

		return err;
	}

private:
	const Char* m_ptr = nullptr;
	mutable U32 m_length = 0;

	/// Constructor for friends
	CString(const Char* ptr, U32 length) 
	:	m_ptr(ptr),
		m_length(length)
	{
		checkInit();
		ANKI_ASSERT(std::strlen(ptr) == length);
	}

	void checkInit() const 
	{
		ANKI_ASSERT(m_ptr != nullptr);
	}
};

template<typename TAlloc>
using StringScopeDestroyer = ScopeDestroyer<StringBase<TAlloc>, TAlloc>;

/// The base class for strings.
template<typename TAlloc>
class StringBase: public NonCopyable
{
public:
	using Char = char; ///< Character type
	using Allocator = TAlloc; ///< Allocator type
	using Self = StringBase;
	using CStringType = CString;
	using Iterator = Char*;
	using ConstIterator = const Char*;

	using ScopeDestroyer = StringScopeDestroyer<Allocator>;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	/// Default constructor.
	StringBase() 
	{}

	/// Move constructor.
	StringBase(StringBase&& b) 
	:	m_data(std::move(b.m_data))
	{}

	/// Requires manual destruction.
	~StringBase() 
	{}

	/// Initialize using a const string.
	ANKI_USE_RESULT Error create(Allocator alloc, const CStringType& cstr);

	/// Initialize using a range. Copies the range of [first, last)
	ANKI_USE_RESULT Error create(Allocator alloc,
		ConstIterator first, ConstIterator last);

	/// Initialize using a character.
	ANKI_USE_RESULT Error create(Allocator alloc, Char c, PtrSize length);

	/// Copy one string to this one.
	ANKI_USE_RESULT Error create(Allocator alloc, const Self& b)
	{
		return create(alloc, b.toCString());
	}

	/// Destroy the string.
	void destroy(Allocator alloc)
	{
		m_data.destroy(alloc);
	}

	/// Move one string to this one.
	Self& operator=(Self&& b) 
	{
		ANKI_ASSERT(this != &b);
		m_data = std::move(b.m_data);
		return *this;
	}

	/// Return char at the specified position.
	const Char& operator[](U pos) const 
	{
		checkInit();
		return m_data[pos];
	}

	/// Return char at the specified position as a modifiable reference.
	Char& operator[](U pos) 
	{
		checkInit();
		return m_data[pos];
	}

	Iterator begin() 
	{
		checkInit();
		return &m_data[0];
	}

	ConstIterator begin() const 
	{
		checkInit();
		return &m_data[0];
	}

	Iterator end() 
	{
		checkInit();
		return &m_data[m_data.getSize() - 1];
	}

	ConstIterator end() const 
	{
		checkInit();
		return &m_data[m_data.getSize() - 1];
	}

	/// Return true if strings are equal
	Bool operator==(const Self& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) == 0;
	}

	/// Return true if strings are not equal
	Bool operator!=(const Self& b) const 
	{
		return !(*this == b);
	}

	/// Return true if this is less than b
	Bool operator<(const Self& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) < 0;
	}

	/// Return true if this is less or equal to b
	Bool operator<=(const Self& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) <= 0;
	}

	/// Return true if this is greater than b
	Bool operator>(const Self& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) > 0;
	}

	/// Return true if this is greater or equal to b
	Bool operator>=(const Self& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) >= 0;
	}

	/// Return true if strings are equal
	Bool operator==(const CStringType& cstr) const 
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) == 0;
	}

	/// Return true if strings are not equal
	Bool operator!=(const CStringType& cstr) const 
	{
		return !(*this == cstr);
	}

	/// Return true if this is less than cstr.
	Bool operator<(const CStringType& cstr) const 
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) < 0;
	}

	/// Return true if this is less or equal to cstr.
	Bool operator<=(const CStringType& cstr) const 
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) <= 0;
	}

	/// Return true if this is greater than cstr.
	Bool operator>(const CStringType& cstr) const 
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) > 0;
	}

	/// Return true if this is greater or equal to cstr.
	Bool operator>=(const CStringType& cstr) const 
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) >= 0;
	}

	/// Return the string's length. It doesn't count the terminating character.
	PtrSize getLength() const 
	{
		auto size = m_data.getSize();
		auto out = (size != 0) ? (size - 1) : 0;
		ANKI_ASSERT(std::strlen(&m_data[0]) == out);
		return out;
	}

	/// Return the CString.
	CStringType toCString() const 
	{
		checkInit();
		return CStringType(&m_data[0], getLength());
	}

	/// Append another string to this one.
	template<typename TTAlloc>
	ANKI_USE_RESULT Error append(Allocator alloc, const StringBase<TTAlloc>& b)
	{
		Error err = ErrorCode::NONE;
		if(!b.isEmpty())
		{
			err = appendInternal(alloc, &b.m_data[0], b.m_data.getSize());
		}

		return err;
	}

	/// Append a const string to this one.
	ANKI_USE_RESULT Error append(Allocator alloc, const CStringType& cstr)
	{
		Error err = ErrorCode::NONE;
		if(!cstr.isEmpty())
		{
			err = appendInternal(alloc, &cstr[0], cstr.getLength() + 1);
		}

		return err;
	}

	/// Create formated string.
	ANKI_USE_RESULT Error sprintf(Allocator alloc, CString fmt, ...);

	/// Return true if it's empty.
	Bool isEmpty() const 
	{
		return m_data.isEmpty();
	}

	/// Find a substring of this string.
	/// @param[in] cstr The substring to search.
	/// @param position Position of the first character in the string to be 
	///                 considered in the search.
	/// @return A valid position if the string is found or NPOS if not found.
	PtrSize find(const CStringType& cstr, PtrSize position = 0) const 
	{
		checkInit();
		return toCString().find(cstr, position);
	}

	/// Find a substring of this string.
	/// @param[in] str The substring to search.
	/// @param position Position of the first character in the string to be 
	///                 considered in the search.
	/// @return A valid position if the string is found or NPOS if not found.
	template<typename TTAlloc>
	PtrSize find(
		const StringBase<TTAlloc>& str, PtrSize position) const 
	{
		str.chechInit();
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	template<typename TNumber>
	ANKI_USE_RESULT Error toString(Allocator alloc, TNumber number);

	/// Convert to F64.
	ANKI_USE_RESULT Error toF64(F64& out) const
	{
		return toCString().toF64(out);
	}

	/// Convert to I64.
	ANKI_USE_RESULT Error toI64(I64& out) const
	{
		return toCString().toI64(out);
	}

private:
	DArray<Char, Allocator> m_data;

	void checkInit() const
	{
		ANKI_ASSERT(m_data.getSize() > 1);
	}

	/// Append to this string.
	ANKI_USE_RESULT Error appendInternal(
		Allocator alloc, const Char* str, PtrSize strSize);
};

/// A common string type that uses heap allocator.
using String = StringBase<HeapAllocator<char>>;

/// @}

} // end namespace anki

#include "anki/util/String.inl.h"

#endif
