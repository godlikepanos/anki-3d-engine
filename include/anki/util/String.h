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

	// For the secret constructor
	friend CString operator"" _cstr(const char*, unsigned long); 

public:
	using Char = char;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	CString() noexcept = default;

	CString(const Char* ptr) noexcept
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

	/// Return true if the string is initialized.
	operator Bool() const noexcept
	{
		return !isEmpty();
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

	/// Return true if the string is not initialized.
	Bool isEmpty() const noexcept
	{
		return m_ptr == nullptr;
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

	PtrSize find(const CString& cstr, PtrSize position = 0) const noexcept
	{
		checkInit();
		ANKI_ASSERT(position < getLength());
		const Char* out = std::strstr(m_ptr, &cstr[0]);
		return (out == nullptr) ? NPOS : (out - m_ptr);
	}

	/// Convert to F64.
	F64 toF64() const
	{
		checkInit();
		F64 out = std::strtod(m_ptr, nullptr);

		if(out == HUGE_VAL)
		{
			throw ANKI_EXCEPTION("Conversion failed");
		}

		return out;
	}

	/// Convert to I64.
	I64 toI64() const
	{
		checkInit();
		I64 out = std::strtoll(m_ptr, nullptr, 10);

		if(out == LLONG_MAX)
		{
			throw ANKI_EXCEPTION("Conversion failed");
		}

		return out;
	}

private:
	const Char* m_ptr = nullptr;
	mutable U16 m_length = 0;

	/// Constructor for friends
	CString(const Char* ptr, U16 length) noexcept
	:	m_ptr(ptr),
		m_length(length)
	{
		checkInit();
		ANKI_ASSERT(std::strlen(ptr) == length);
	}

	void checkInit() const noexcept
	{
		ANKI_ASSERT(m_ptr != nullptr);
	}
};

/// User defined string literal for CStrings.
CString operator"" _cstr(const char* str, unsigned long length)
{
	return CString(str, length);
}

/// The base class for strings.
template<typename TAlloc>
class StringBase
{
public:
	using Char = char; ///< Character type
	using Allocator = TAlloc; ///< Allocator type
	using Self = StringBase;
	using CStringType = CString;
	using Iterator = Char*;
	using ConstIterator = const Char*;

	static const PtrSize NPOS = MAX_PTR_SIZE;

	StringBase() noexcept
	{}

	StringBase(Allocator alloc) noexcept
	:	m_data(alloc)
	{}

	/// Initialize using a const string.
	StringBase(const CStringType& cstr, Allocator alloc)
	:	m_data(alloc)
	{
		auto size = cstr.getLength() + 1;
		m_data.resize(size);
		std::memcpy(&m_data[0], cstr.get(), sizeof(Char) * size);
	}

	/// Initialize using a range. Copies the range of [first, last)
	StringBase(ConstIterator first, ConstIterator last, Allocator alloc)
	:	m_data(alloc)
	{
		ANKI_ASSERT(first != 0 && last != 0);
		auto length = last - first;
		m_data.resize(length + 1);
		std::memcpy(&m_data[0], first, length);
		m_data[length] = '\0';
	}

	/// Copy constructor.
	StringBase(const Self& b)
	:	m_data(b.m_data)
	{}

	/// Move constructor.
	StringBase(StringBase&& b) noexcept
	:	m_data(b.m_data)
	{}

	/// Destroys the string.
	~StringBase() noexcept
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
	const Char& operator[](U pos) const noexcept
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

	Iterator begin() noexcept
	{
		checkInit();
		return &m_data[0];
	}

	ConstIterator begin() const noexcept
	{
		checkInit();
		return &m_data[0];
	}

	Iterator end() noexcept
	{
		checkInit();
		return &m_data[m_data.size() - 1];
	}

	ConstIterator end() const noexcept
	{
		checkInit();
		return &m_data[m_data.size() - 1];
	}

	/// Add strings.
	Self operator+(const Self& b) const
	{
		b.checkInit();
		return add(&b.m_data[0], b.m_data.size());
	}

	/// Add strings.
	Self operator+(const CString& cstr) const
	{
		return add(&cstr[0], cstr.getLength() + 1);
	}

	/// Append another string to this one.
	template<typename TTAlloc>
	Self& operator+=(const StringBase<TTAlloc>& b)
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

	/// Return the allocator
	Allocator getAllocator() const noexcept
	{
		return m_data.get_allocator();
	}

	/// Return the CString.
	CStringType toCString() const noexcept
	{
		checkInit();
		return CStringType(&m_data[0], getLength());
	}

	Self& sprintf(CString fmt, ...)
	{
		Array<Char, 512> buffer;
		va_list args;
		Char* out = &buffer[0];

		va_start(args, fmt);
		I len = std::vsnprintf(&buffer[0], sizeof(buffer), &fmt[0], args);
		va_end(args);

		if(len < 0)
		{
			throw ANKI_EXCEPTION("vsnprintf() failed");
		}
		else if(static_cast<PtrSize>(len) >= sizeof(buffer))
		{
			I size = len + 1;
			out = reinterpret_cast<Char*>(getAllocator().allocate(size));

			va_start(args, fmt);
			len = std::vsnprintf(out, size, &fmt[0], args);
			(void)len;
			va_end(args);

			ANKI_ASSERT(len < size);
		}

		*this = out;

		// Delete the allocated memory
		if(out != &buffer[0])
		{
			getAllocator().deallocate(out, 0);
		}

		return *this;
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

	/// Resize the string.
	void resize(PtrSize newLength, const Char c = '\0')
	{
		ANKI_ASSERT(newLength > 0);
		auto size = m_data.size();
		m_data.resize(newLength + 1);

		if(size < newLength + 1)
		{
			// The string has grown
			
			// Fill the extra space with c
			std::memset(&m_data[size - 1], c, newLength - size + 1);
			m_data[newLength] = '\0';
		}
		else if(size > newLength + 1)
		{
			// The string got shrinked

			m_data[newLength] = '\0';
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
	PtrSize find(const CStringType& cstr, PtrSize position = 0) const noexcept
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
		const StringBase<TTAlloc>& str, PtrSize position) const noexcept
	{
		str.chechInit();
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	/// @param number The number to convert.
	/// @param[in,out] alloc The allocator to allocate the returned string.
	/// @return The string that presents the number.
	template<typename TNumber, typename TTAlloc>
	static Self toString(TNumber number, TTAlloc alloc)
	{
		Array<Char, 512> buff;
		I ret = std::snprintf(
			&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

		if(ret < 0 || ret > static_cast<I>(buff.size()))
		{
			throw ANKI_EXCEPTION("To small intermediate buffer");
		}

		using YAlloc = typename TTAlloc::template rebind<Char>::other;
		return StringBase<YAlloc>(&buff[0], alloc);
	}

	/// Convert to F64.
	F64 toF64() const
	{
		return toCString().toF64();
	}

	/// Convert to I64.
	I64 toI64() const
	{
		return toCString().toI64();
	}

private:
	Vector<Char, TAlloc> m_data;

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

	Self add(const Char* str, PtrSize strSize) const
	{
		checkInit();

		Self out(m_data.get_allocator());
		out.m_data.resize(m_data.size() + strSize - 1);

		std::memcpy(&out.m_data[0], &m_data[0], 
			(m_data.size() - 1) * sizeof(Char));
		std::memcpy(&out.m_data[m_data.size() - 1], str, 
			strSize * sizeof(Char));

		return out;
	}
};

template<typename TAlloc>
inline StringBase<TAlloc> operator+(
	const CString& left, const StringBase<TAlloc>& right)
{
	StringBase<TAlloc> out(right.getAllocator());

	auto leftLength = left.getLength();
	out.resize(leftLength + right.getLength());

	std::memcpy(&out[0], &left[0], leftLength);
	std::memcpy(&out[leftLength], &right[0], right.getLength() + 1);

	return out;
}

/// A common string type that uses heap allocator.
using String = StringBase<HeapAllocator<char>>;

/// @}

} // end namespace anki

#endif
