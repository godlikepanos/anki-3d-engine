// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
class String;

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
	friend class String; // For the secret constructor

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

/// The base class for strings.
class String: public NonCopyable
{
public:
	using Char = char; ///< Character type
	using CStringType = CString;
	using Iterator = Char*;
	using ConstIterator = const Char*;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	/// Default constructor.
	String() 
	{}

	/// Move constructor.
	String(String&& b) 
	{
		move(b);
	}

	/// Requires manual destruction.
	~String() 
	{}

	/// Initialize using a const string.
	template<typename TAllocator>
	void create(TAllocator alloc, const CStringType& cstr);

	/// Initialize using a range. Copies the range of [first, last)
	template<typename TAllocator>
	void create(TAllocator alloc,
		ConstIterator first, ConstIterator last);

	/// Initialize using a character.
	template<typename TAllocator>
	void create(TAllocator alloc, Char c, PtrSize length);

	/// Copy one string to this one.
	template<typename TAllocator>
	void create(TAllocator alloc, const String& b)
	{
		create(alloc, b.toCString());
	}

	/// Destroy the string.
	template<typename TAllocator>
	void destroy(TAllocator alloc)
	{
		m_data.destroy(alloc);
	}

	/// Move one string to this one.
	String& operator=(String&& b) 
	{
		move(b);
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
	Bool operator==(const String& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) == 0;
	}

	/// Return true if strings are not equal
	Bool operator!=(const String& b) const 
	{
		return !(*this == b);
	}

	/// Return true if this is less than b
	Bool operator<(const String& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) < 0;
	}

	/// Return true if this is less or equal to b
	Bool operator<=(const String& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) <= 0;
	}

	/// Return true if this is greater than b
	Bool operator>(const String& b) const 
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) > 0;
	}

	/// Return true if this is greater or equal to b
	Bool operator>=(const String& b) const 
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
	template<typename TAllocator>
	void append(TAllocator alloc, const String& b)
	{
		if(!b.isEmpty())
		{
			appendInternal(alloc, &b.m_data[0], b.m_data.getSize());
		}
	}

	/// Append a const string to this one.
	template<typename TAllocator>
	void append(TAllocator alloc, const CStringType& cstr)
	{
		if(!cstr.isEmpty())
		{
			appendInternal(alloc, &cstr[0], cstr.getLength() + 1);
		}
	}

	/// Create formated string.
	template<typename TAllocator>
	void sprintf(TAllocator alloc, CString fmt, ...);

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
	PtrSize find(const String& str, PtrSize position) const 
	{
		str.checkInit();
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	template<typename TAllocator, typename TNumber>
	void toString(TAllocator alloc, TNumber number);

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

protected:
	DArray<Char> m_data;

	void checkInit() const
	{
		ANKI_ASSERT(m_data.getSize() > 1);
	}

	/// Append to this string.
	template<typename TAllocator>
	void appendInternal(TAllocator alloc, const Char* str, PtrSize strSize);

	void move(String& b)
	{
		ANKI_ASSERT(this != &b);
		m_data = std::move(b.m_data);
	}
};

/// String with automatic cleanup.
class StringAuto: public String
{
public:
	using Base = String;

	/// Create with allocator.
	template<typename TAllocator>
	StringAuto(TAllocator alloc)
	:	Base(),
		m_alloc(alloc)
	{}

	/// Move constructor.
	StringAuto(StringAuto&& b)
	:	Base()
	{
		move(b);
	}

	/// Automatic destruction.
	~StringAuto() 
	{
		Base::destroy(m_alloc);
	}

	/// Initialize using a const string.
	void create(const CStringType& cstr)
	{
		Base::create(m_alloc, cstr);
	}

	/// Initialize using a range. Copies the range of [first, last)
	void create(ConstIterator first, ConstIterator last)
	{
		Base::create(m_alloc, first, last);
	}

	/// Initialize using a character.
	void create(Char c, PtrSize length)
	{
		Base::create(m_alloc, c, length);
	}

	/// Copy one string to this one.
	void create(const String& b)
	{
		Base::create(m_alloc, b.toCString());
	}

	/// Move one string to this one.
	StringAuto& operator=(StringAuto&& b) 
	{
		move(b);
		return *this;
	}

	/// Append another string to this one.
	void append(const String& b)
	{
		Base::append(m_alloc, b);
	}

	/// Append a const string to this one.
	void append(const CStringType& cstr)
	{
		Base::append(m_alloc, cstr);
	}

	/// Create formated string.
	template<typename... TArgs>
	void sprintf(CString fmt, TArgs... args)
	{
		Base::sprintf(m_alloc, fmt, args...);
	}

	/// Convert a number to a string.
	template<typename TNumber>
	void toString(TNumber number)
	{
		Base::toString(m_alloc, number);
	}

private:
	GenericMemoryPoolAllocator<Char> m_alloc;

	void move(StringAuto& b)
	{
		Base::move(b);
		m_alloc = std::move(b.m_alloc);
	}
};
/// @}

} // end namespace anki

#include "anki/util/String.inl.h"

#endif
