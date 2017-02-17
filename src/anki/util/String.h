// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/DynamicArray.h>
#include <anki/util/Array.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Hash.h>
#include <cstring>
#include <cstdio>
#include <cinttypes> // For PRId8 etc

namespace anki
{

// Forward
class String;
class StringAuto;

/// @addtogroup util_private
/// @{

namespace detail
{

template<typename TNumber>
constexpr const char* toStringFormat()
{
	return nullptr;
}

#define ANKI_DEPLOY_TO_STRING(type_, string_)                                                                          \
	template<>                                                                                                         \
	constexpr const char* toStringFormat<type_>()                                                                      \
	{                                                                                                                  \
		return string_;                                                                                                \
	}

ANKI_DEPLOY_TO_STRING(I8, "%" PRId8)
ANKI_DEPLOY_TO_STRING(I16, "%" PRId16)
ANKI_DEPLOY_TO_STRING(I32, "%" PRId32)
ANKI_DEPLOY_TO_STRING(I64, "%" PRId64)
ANKI_DEPLOY_TO_STRING(U8, "%" PRIu8)
ANKI_DEPLOY_TO_STRING(U16, "%" PRIu16)
ANKI_DEPLOY_TO_STRING(U32, "%" PRIu32)
ANKI_DEPLOY_TO_STRING(U64, "%" PRIu64)
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
public:
	using Char = char;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	CString() = default;

	CString(const Char* ptr)
		: m_ptr(ptr)
	{
	}

	/// Copy constructor.
	CString(const CString& b)
		: m_ptr(b.m_ptr)
	{
	}

	/// Copy.
	CString& operator=(const CString& b)
	{
		m_ptr = b.m_ptr;
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
	U32 getLength() const
	{
		checkInit();
		return U32(std::strlen(m_ptr));
	}

	PtrSize find(const CString& cstr, PtrSize position = 0) const
	{
		checkInit();
		ANKI_ASSERT(position < getLength());
		const Char* out = std::strstr(m_ptr, &cstr[0]);
		return (out == nullptr) ? NPOS : PtrSize(out - m_ptr);
	}

	/// Convert to F64.
	ANKI_USE_RESULT Error toF64(F64& out) const;

	/// Convert to F32.
	ANKI_USE_RESULT Error toF32(F32& out) const;

	/// Convert to I64.
	ANKI_USE_RESULT Error toI64(I64& out) const;

private:
	const Char* m_ptr = nullptr;

	void checkInit() const
	{
		ANKI_ASSERT(m_ptr != nullptr);
	}
};

/// Hasher function for CStrings. Can be used in HashMap.
class CStringHasher
{
public:
	U64 operator()(CString str)
	{
		ANKI_ASSERT(!str.isEmpty());
		return computeHash(&str[0], str.getLength());
	}
};

/// Compare function for CStrings. Can be used in HashMap.
class CStringCompare
{
public:
	Bool operator()(CString a, CString b)
	{
		return a == b;
	}
};

/// The base class for strings.
class String : public NonCopyable
{
public:
	using Char = char; ///< Character type
	using CStringType = CString;
	using Iterator = Char*;
	using ConstIterator = const Char*;
	using Allocator = GenericMemoryPoolAllocator<Char>;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	/// Default constructor.
	String()
	{
	}

	/// Move constructor.
	String(String&& b)
	{
		*this = std::move(b);
	}

	String(StringAuto&& b)
	{
		*this = std::move(b);
	}

	/// Requires manual destruction.
	~String()
	{
	}

	/// Initialize using a const string.
	void create(Allocator alloc, const CStringType& cstr);

	/// Initialize using a range. Copies the range of [first, last)
	void create(Allocator alloc, ConstIterator first, ConstIterator last);

	/// Initialize using a character.
	void create(Allocator alloc, Char c, PtrSize length);

	/// Copy one string to this one.
	void create(Allocator alloc, const String& b)
	{
		create(alloc, b.toCString());
	}

	/// Destroy the string.
	void destroy(Allocator alloc)
	{
		m_data.destroy(alloc);
	}

	/// Move one string to this one.
	String& operator=(String&& b)
	{
		move(b);
		return *this;
	}

	/// Move a StringAuto to this one.
	String& operator=(StringAuto&& b);

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
		return CStringType(&m_data[0]);
	}

	/// Append another string to this one.
	void append(Allocator alloc, const String& b)
	{
		if(!b.isEmpty())
		{
			appendInternal(alloc, &b.m_data[0], b.m_data.getSize());
		}
	}

	/// Append a const string to this one.
	void append(Allocator alloc, const CStringType& cstr)
	{
		if(!cstr.isEmpty())
		{
			appendInternal(alloc, &cstr[0], cstr.getLength() + 1);
		}
	}

	/// Create formated string.
	void sprintf(Allocator alloc, CString fmt, ...);

	/// Return true if it's empty.
	Bool isEmpty() const
	{
		return m_data.isEmpty();
	}

	/// Find a substring of this string.
	/// @param[in] cstr The substring to search.
	/// @param position Position of the first character in the string to be considered in the search.
	/// @return A valid position if the string is found or NPOS if not found.
	PtrSize find(const CStringType& cstr, PtrSize position = 0) const
	{
		checkInit();
		return toCString().find(cstr, position);
	}

	/// Find a substring of this string.
	/// @param[in] str The substring to search.
	/// @param position Position of the first character in the string to be considered in the search.
	/// @return A valid position if the string is found or NPOS if not found.
	PtrSize find(const String& str, PtrSize position) const
	{
		str.checkInit();
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	template<typename TNumber>
	void toString(Allocator alloc, TNumber number);

	/// Convert to F64.
	ANKI_USE_RESULT Error toF64(F64& out) const
	{
		return toCString().toF64(out);
	}

	/// Convert to F32.
	ANKI_USE_RESULT Error toF32(F32& out) const
	{
		return toCString().toF32(out);
	}

	/// Convert to I64.
	ANKI_USE_RESULT Error toI64(I64& out) const
	{
		return toCString().toI64(out);
	}

protected:
	DynamicArray<Char> m_data;

	void checkInit() const
	{
		ANKI_ASSERT(m_data.getSize() > 1);
	}

	/// Append to this string.
	void appendInternal(Allocator alloc, const Char* str, PtrSize strSize);

	void move(String& b)
	{
		ANKI_ASSERT(this != &b);
		m_data = std::move(b.m_data);
	}
};

template<typename TNumber>
inline void String::toString(Allocator alloc, TNumber number)
{
	destroy(alloc);

	Array<Char, 512> buff;
	I ret = std::snprintf(&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

	if(ret < 0 || ret > static_cast<I>(buff.getSize()))
	{
		ANKI_UTIL_LOGF("To small intermediate buffer");
	}
	else
	{
		create(alloc, &buff[0]);
	}
}

/// String with automatic cleanup.
class StringAuto : public String
{
public:
	using Base = String;
	using Allocator = typename Base::Allocator;

	/// Create with allocator.
	StringAuto(Allocator alloc)
		: Base()
		, m_alloc(alloc)
	{
	}

	/// Move constructor.
	StringAuto(StringAuto&& b)
		: Base()
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

	/// Destroy the string.
	void destroy()
	{
		Base::destroy(m_alloc);
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
