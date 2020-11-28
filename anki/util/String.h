// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/DynamicArray.h>
#include <anki/util/Array.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Hash.h>
#include <anki/util/Forward.h>
#include <cstring>
#include <cstdio>
#include <cinttypes> // For PRId8 etc

namespace anki
{

// Forward
class F16;

/// @addtogroup util_private
/// @{

namespace detail
{

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

	static constexpr PtrSize NPOS = MAX_PTR_SIZE;

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
	explicit operator Bool() const
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

	/// Get C-string.
	const Char* cstr() const
	{
		checkInit();
		return m_ptr;
	}

	const Char* getBegin() const
	{
		checkInit();
		return &m_ptr[0];
	}

	const Char* getEnd() const
	{
		checkInit();
		return &m_ptr[getLength()];
	}

	const Char* begin() const
	{
		return getBegin();
	}

	const Char* end() const
	{
		return getEnd();
	}

	/// Return true if the string is not initialized.
	Bool isEmpty() const
	{
		return m_ptr == nullptr || getLength() == 0;
	}

	/// Get the string length.
	U32 getLength() const
	{
		return (m_ptr == nullptr) ? 0 : U32(std::strlen(m_ptr));
	}

	PtrSize find(const CString& cstr, PtrSize position = 0) const
	{
		checkInit();
		ANKI_ASSERT(position < getLength());
		const Char* out = std::strstr(m_ptr + position, &cstr[0]);
		return (out == nullptr) ? NPOS : PtrSize(out - m_ptr);
	}

	/// Convert to F16.
	ANKI_USE_RESULT Error toNumber(F16& out) const;

	/// Convert to F32.
	ANKI_USE_RESULT Error toNumber(F32& out) const;

	/// Convert to F64.
	ANKI_USE_RESULT Error toNumber(F64& out) const;

	/// Convert to I8.
	ANKI_USE_RESULT Error toNumber(I8& out) const;

	/// Convert to U8.
	ANKI_USE_RESULT Error toNumber(U8& out) const;

	/// Convert to I16.
	ANKI_USE_RESULT Error toNumber(I16& out) const;

	/// Convert to U16.
	ANKI_USE_RESULT Error toNumber(U16& out) const;

	/// Convert to I32.
	ANKI_USE_RESULT Error toNumber(I32& out) const;

	/// Convert to U32.
	ANKI_USE_RESULT Error toNumber(U32& out) const;

	/// Convert to I64.
	ANKI_USE_RESULT Error toNumber(I64& out) const;

	/// Convert to U64.
	ANKI_USE_RESULT Error toNumber(U64& out) const;

	/// Convert to Bool.
	ANKI_USE_RESULT Error toNumber(Bool& out) const;

	/// Compute the hash.
	U64 computeHash() const
	{
		checkInit();
		return anki::computeHash(m_ptr, getLength());
	}

private:
	const Char* m_ptr = nullptr;

	void checkInit() const
	{
		ANKI_ASSERT(m_ptr != nullptr);
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

	String(Allocator alloc, CString str)
	{
		create(alloc, str);
	}

	/// Requires manual destruction.
	~String()
	{
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

	explicit operator Bool() const
	{
		return !isEmpty();
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

	/// Get a C string.
	const Char* cstr() const
	{
		checkInit();
		return &m_data[0];
	}

	operator CString() const
	{
		return toCString();
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

	/// Append another string to this one.
	String& append(Allocator alloc, const String& b)
	{
		if(!b.isEmpty())
		{
			appendInternal(alloc, &b.m_data[0], b.m_data.getSize() - 1);
		}
		return *this;
	}

	/// Append a const string to this one.
	String& append(Allocator alloc, const CStringType& cstr)
	{
		if(!cstr.isEmpty())
		{
			appendInternal(alloc, cstr.cstr(), cstr.getLength());
		}
		return *this;
	}

	/// Append using a range. Copies the range of [first, oneAfterLast)
	String& append(Allocator alloc, ConstIterator first, ConstIterator oneAfterLast)
	{
		const PtrSize len = oneAfterLast - first;
		appendInternal(alloc, first, len);
		return *this;
	}

	/// Create formated string.
	String& sprintf(Allocator alloc, CString fmt, ...);

	/// Destroy the string.
	void destroy(Allocator alloc)
	{
		m_data.destroy(alloc);
	}

	Iterator getBegin()
	{
		checkInit();
		return &m_data[0];
	}

	ConstIterator getBegin() const
	{
		checkInit();
		return &m_data[0];
	}

	Iterator getEnd()
	{
		checkInit();
		return &m_data[m_data.getSize() - 1];
	}

	ConstIterator getEnd() const
	{
		checkInit();
		return &m_data[m_data.getSize() - 1];
	}

	Iterator begin()
	{
		return getBegin();
	}

	ConstIterator begin() const
	{
		return getBegin();
	}

	Iterator end()
	{
		return getEnd();
	}

	ConstIterator end() const
	{
		return getEnd();
	}

	/// Return the string's length. It doesn't count the terminating character.
	U32 getLength() const
	{
		return (m_data.getSize() == 0) ? 0 : U32(std::strlen(&m_data[0]));
	}

	/// Return the CString.
	CStringType toCString() const
	{
		return (!isEmpty()) ? CStringType(&m_data[0]) : CStringType();
	}

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

	/// Convert to F16.
	ANKI_USE_RESULT Error toNumber(F16& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to F32.
	ANKI_USE_RESULT Error toNumber(F32& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to F64.
	ANKI_USE_RESULT Error toNumber(F64& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I8.
	ANKI_USE_RESULT Error toNumber(I8& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U8.
	ANKI_USE_RESULT Error toNumber(U8& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I16.
	ANKI_USE_RESULT Error toNumber(I16& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U16.
	ANKI_USE_RESULT Error toNumber(U16& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I32.
	ANKI_USE_RESULT Error toNumber(I32& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U32.
	ANKI_USE_RESULT Error toNumber(U32& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I64.
	ANKI_USE_RESULT Error toNumber(I64& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U64.
	ANKI_USE_RESULT Error toNumber(U64& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to Bool.
	ANKI_USE_RESULT Error toNumber(Bool& out) const
	{
		return toCString().toNumber(out);
	}

	/// Compute the hash.
	U64 computeHash() const
	{
		checkInit();
		return anki::computeHash(&m_data[0], m_data.getSize());
	}

	/// Replace all occurrences of "from" with "to".
	String& replaceAll(Allocator alloc, CString from, CString to);

	/// @brief  Execute a functor for all characters of the string.
	template<typename TFunc>
	void transform(TFunc func)
	{
		U i = 0;
		while(i < m_data.getSize() && m_data[i] != '\0')
		{
			func(m_data[i]);
			++i;
		}
	}

protected:
	DynamicArray<Char, PtrSize> m_data;

	void checkInit() const
	{
		ANKI_ASSERT(m_data.getSize() > 0);
	}

	/// Append to this string.
	void appendInternal(Allocator& alloc, const Char* str, PtrSize strLen);

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
	const I ret = std::snprintf(&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

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

	/// Copy construcor.
	StringAuto(const StringAuto& b)
		: Base()
		, m_alloc(b.m_alloc)
	{
		if(!b.isEmpty())
		{
			create(b.m_data.getBegin(), b.m_data.getEnd());
		}
	}

	/// Move constructor.
	StringAuto(StringAuto&& b)
		: Base()
	{
		move(b);
	}

	/// Create with allocator and data.
	StringAuto(Allocator alloc, const CStringType& cstr)
		: Base()
		, m_alloc(alloc)
	{
		create(cstr);
	}

	/// Automatic destruction.
	~StringAuto()
	{
		Base::destroy(m_alloc);
	}

	/// Copy operator.
	StringAuto& operator=(const StringAuto& b)
	{
		destroy();
		m_alloc = b.m_alloc;
		if(!b.isEmpty())
		{
			create(b.m_data.getBegin(), b.m_data.getEnd());
		}
		return *this;
	}

	/// Copy from string.
	StringAuto& operator=(const CString& b)
	{
		destroy();
		if(!b.isEmpty())
		{
			create(b.getBegin(), b.getEnd());
		}
		return *this;
	}

	/// Move one string to this one.
	StringAuto& operator=(StringAuto&& b)
	{
		destroy();
		move(b);
		return *this;
	}

	GenericMemoryPoolAllocator<Char> getAllocator() const
	{
		return m_alloc;
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

	/// Append another string to this one.
	StringAuto& append(const String& b)
	{
		Base::append(m_alloc, b);
		return *this;
	}

	/// Append a const string to this one.
	StringAuto& append(const CStringType& cstr)
	{
		Base::append(m_alloc, cstr);
		return *this;
	}

	/// Create formated string.
	template<typename... TArgs>
	StringAuto& sprintf(CString fmt, TArgs... args)
	{
		Base::sprintf(m_alloc, fmt, args...);
		return *this;
	}

	/// Convert a number to a string.
	template<typename TNumber>
	void toString(TNumber number)
	{
		Base::toString(m_alloc, number);
	}

	/// Replace all occurrences of "from" with "to".
	StringAuto& replaceAll(CString from, CString to)
	{
		Base::replaceAll(m_alloc, from, to);
		return *this;
	}

private:
	GenericMemoryPoolAllocator<Char> m_alloc;

	void move(StringAuto& b)
	{
		Base::move(b);
		m_alloc = std::move(b.m_alloc);
	}
};

#define ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, op) \
	inline Bool operator op(TypeA a, TypeB b) \
	{ \
		return CString(a) op CString(b); \
	}

#define ANKI_STRING_COMPARE_OPS(TypeA, TypeB) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, ==) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, !=) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, <) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, <=) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, >) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, >=)

ANKI_STRING_COMPARE_OPS(const char*, CString)
ANKI_STRING_COMPARE_OPS(const char*, const String&)
ANKI_STRING_COMPARE_OPS(const char*, const StringAuto&)

ANKI_STRING_COMPARE_OPS(CString, const char*)
ANKI_STRING_COMPARE_OPS(CString, const String&)
ANKI_STRING_COMPARE_OPS(CString, const StringAuto&)

ANKI_STRING_COMPARE_OPS(const String&, const char*)
ANKI_STRING_COMPARE_OPS(const String&, CString)
ANKI_STRING_COMPARE_OPS(const String&, const StringAuto&)

ANKI_STRING_COMPARE_OPS(const StringAuto&, const char*)
ANKI_STRING_COMPARE_OPS(const StringAuto&, CString)
ANKI_STRING_COMPARE_OPS(const StringAuto&, const String&)
ANKI_STRING_COMPARE_OPS(const StringAuto&, const StringAuto&)

#undef ANKI_STRING_COMPARE_OPERATOR
#undef ANKI_STRING_COMPARE_OPS
/// @}

} // end namespace anki
