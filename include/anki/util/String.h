// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_STRING_H
#define ANKI_STRING_H

#include "anki/util/Vector.h"
#include "anki/util/Exception.h"
#include "anki/util/Array.h"
#include <cstring>

namespace anki {

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
public:
	using Char = char;

	CString() noexcept = default;

	explicit CString(const Char* ptr) noexcept
	:	m_ptr(ptr)
	{
		checkInit();
	}

	/// Copy constructor.
	CString(const CString& b) noexcept
	:	m_ptr(b.m_ptr),
		m_length(b.m_length)
	{
		checkInit();
	}

	/// Copy.
	CString& operator=(const CString& b) noexcept
	{
		m_ptr = b.m_ptr;
		m_length = b.m_length;
		checkInit();
		return *this;
	}

	/// Return char at the specified position.
	const Char& operator[](U pos) const noexcept
	{
		checkInit();
		ANKI_ASSERT(pos <= getLength());
		return m_ptr[pos];
	}

	const Char* begin() const noexcept
	{
		checkInit();
		return &m_ptr[0];
	}

	const Char* end() const noexcept
	{
		checkInit();
		return &m_ptr[getLength()];
	}

	Bool operator==(const CString& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(m_ptr, b.m_ptr) == 0;
	}

	Bool operator<(const CString& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(m_ptr, b.m_ptr) < 0;
	}

	Bool operator<=(const CString& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(m_ptr, b.m_ptr) <= 0;
	}

	Bool operator>(const CString& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(m_ptr, b.m_ptr) > 0;
	}

	Bool operator>=(const CString& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(m_ptr, b.m_ptr) >= 0;
	}

	/// Get the underlying C string.
	const char* get() const noexcept
	{
		checkInit();
		return m_ptr;
	}

	/// Get the string length.
	U getLength() const noexcept
	{
		checkInit();
		if(m_length == 0)
		{
			m_length = std::strlen(m_ptr);
		}
		
		return m_length;
	}

private:
	const Char* m_ptr = nullptr;
	mutable U16 m_length = 0;

	void checkInit() const noexcept
	{
		ANKI_ASSERT(m_ptr != nullptr);
		ANKI_ASSERT(m_ptr[0] != '\0' && "Empty strings are now allowed");
	}
};

/// The base class for strings.
template<template <typename> class TAlloc>
class BasicString
{
public:
	using Char = char; ///< Character type
	using Allocator = TAlloc<Char>; ///< Allocator type
	using Self = BasicString;
	using CStringType = CString;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	BasicString() noexcept
	{}

	BasicString(Allocator& alloc) noexcept
	:	m_data(alloc)
	{}

	/// Copy constructor.
	BasicString(const Self& b)
	:	m_data(b.m_data)
	{}

	/// Move constructor.
	BasicString(BasicString&& b) noexcept
	:	m_data(b.m_data)
	{}

	/// Initialize using a const string.
	template<typename TTAlloc>
	BasicString(const CStringType& cstr, TTAlloc& alloc)
	:	m_data(alloc)
	{
		auto size = cstr.getLength() + 1;
		m_data.resize(size);
		std::memcpy(&m_data[0], cstr.get(), sizeof(Char) * size);
	}

	/// Destroys the string.
	~BasicString() noexcept
	{}

	/// Copy another string to this one.
	Self& operator=(const Self& b)
	{
		ANKI_ASSERT(this != &b);
		m_data = b.m_data;
		return *this;
	}

	/// Copy a const string to this one.
	Self& operator=(const CStringType& cstr)
	{
		auto size = cstr.getLength() + 1;
		m_data.resize(size);
		std::memcpy(&m_data[0], cstr.get(), sizeof(Char) * size);
		return *this;
	}

	/// Move one string to this one.
	Self& operator=(Self&& b) noexcept
	{
		ANKI_ASSERT(this != &b);
		m_data = std::move(b.m_data);
		return *this;
	}

	/// Return char at the specified position.
	Char operator[](U pos) const noexcept
	{
		checkInit();
		return m_data[pos];
	}

	/// Return char at the specified position as a modifiable reference.
	Char& operator[](U pos) noexcept
	{
		checkInit();
		return m_data[pos];
	}

	Char* begin() noexcept
	{
		checkInit();
		return &m_data[0];
	}

	const Char* begin() const noexcept
	{
		checkInit();
		return &m_data[0];
	}

	Char* end() noexcept
	{
		checkInit();
		return &m_data[m_data.size() - 1];
	}

	const Char* end() const noexcept
	{
		checkInit();
		return &m_data[m_data.size() - 1];
	}

	/// Append another string to this one.
	template<template <typename> class TTAlloc>
	Self& operator+=(const BasicString<TTAlloc>& b)
	{
		b.checkInit();
		append(&b.m_data[0], b.m_data.size());
		return *this;
	}

	/// Append a const string to this one.
	Self& operator+=(const CStringType& cstr)
	{
		U size = cstr.getLength() + 1;
		append(cstr.get(), size);
		return *this;
	}

	/// Return true if strings are equal
	Bool operator==(const Self& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) == 0;
	}

	/// Return true if strings are not equal
	Bool operator!=(const Self& b) const noexcept
	{
		return !(*this == b);
	}

	/// Return true if this is less than b
	Bool operator<(const Self& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) < 0;
	}

	/// Return true if this is less or equal to b
	Bool operator<=(const Self& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) <= 0;
	}

	/// Return true if this is greater than b
	Bool operator>(const Self& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) > 0;
	}

	/// Return true if this is greater or equal to b
	Bool operator>=(const Self& b) const noexcept
	{
		checkInit();
		b.checkInit();
		return std::strcmp(&m_data[0], &b.m_data[0]) >= 0;
	}

	/// Return true if strings are equal
	Bool operator==(const CStringType& cstr) const noexcept
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) == 0;
	}

	/// Return true if strings are not equal
	Bool operator!=(const CStringType& cstr) const noexcept
	{
		return !(*this == cstr);
	}

	/// Return true if this is less than cstr.
	Bool operator<(const CStringType& cstr) const noexcept
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) < 0;
	}

	/// Return true if this is less or equal to cstr.
	Bool operator<=(const CStringType& cstr) const noexcept
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) <= 0;
	}

	/// Return true if this is greater than cstr.
	Bool operator>(const CStringType& cstr) const noexcept
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) > 0;
	}

	/// Return true if this is greater or equal to cstr.
	Bool operator>=(const CStringType& cstr) const noexcept
	{
		checkInit();
		return std::strcmp(&m_data[0], cstr.get()) >= 0;
	}

	/// Return the string's length. It doesn't count the terminating character.
	PtrSize getLength() const noexcept
	{
		auto size = m_data.size();
		return (size != 0) ? (size - 1) : 0;
	}

	/// Return the CString.
	CStringType toCString() const noexcept
	{
		checkInit();
		return CStringType(&m_data[0]);
	}

	/// Return the allocator
	Allocator getAllocator() const noexcept
	{
		return m_data.get_allocator();
	}

	/// Clears the contents of the string and makes it empty.
	void clear()
	{
		m_data.clear();
	}

	/// Request string capacity change.
	void reserve(PtrSize size)
	{
		++size;
		if(size > m_data.size())
		{
			m_data.reserve(size);
		}
	}

	/// Return true if it's empty.
	Bool isEmpty() const noexcept
	{
		return m_data.empty();
	}

	/// Find a substring of this string.
	/// @param[in] cstr The substring to search.
	/// @param position Position of the first character in the string to be 
	///                 considered in the search.
	/// @return A valid position if the string is found or NPOS if not found.
	PtrSize find(const CStringType& cstr, PtrSize position) const noexcept
	{
		checkInit();
		ANKI_ASSERT(position < m_data.size() - 1);
		const Char* out = std::strstr(&m_data[position], cstr.get());
		return (out == nullptr) ? NPOS : (out - &m_data[0]);
	}

	/// Find a substring of this string.
	/// @param[in] str The substring to search.
	/// @param position Position of the first character in the string to be 
	///                 considered in the search.
	/// @return A valid position if the string is found or NPOS if not found.
	template<template <typename> class TTAlloc>
	PtrSize find(
		const BasicString<TTAlloc>& str, PtrSize position) const noexcept
	{
		str.chechInit();
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	/// @param number The number to convert.
	/// @param[in,out] alloc The allocator to allocate the returned string.
	/// @return The string that presents the number.
	template<typename TNumber>
	Self toString(TNumber number, Allocator& alloc)
	{
		Array<Char, 512> buff;
		I ret = std::snprintf(
			&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

		if(ret < 0 || ret > static_cast<I>(buff.size()))
		{
			throw ANKI_EXCEPTION("To small intermediate buffer");
		}

		return Self(alloc, CStringType(&buff[0]));
	}

private:
	Vector<Char, Allocator> m_data;

	void checkInit() const
	{
		ANKI_ASSERT(m_data.size() > 1);
	}

	void append(const Char* str, PtrSize strSize)
	{
		ANKI_ASSERT(str != nullptr);
		ANKI_ASSERT(strSize > 1);

		auto size = m_data.size();

		// Fix empty string
		if(size == 0)
		{
			size = 1;
		}

		m_data.resize(size + strSize - 1);
		std::memcpy(&m_data[size - 1], str, sizeof(Char) * strSize);
	}
};

/// A common string type that uses heap allocator.
using String = BasicString<HeapAllocator>;

/// @}

} // end namespace anki

#endif
