// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/Hash.h>
#include <AnKi/Util/Forward.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cinttypes> // For PRId8 etc
#include <cstdarg> // For var args

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

	static constexpr PtrSize kNpos = kMaxPtrSize;

	CString() = default;

	constexpr CString(const Char* ptr)
		: m_ptr(ptr)
	{
	}

	/// Copy constructor.
	constexpr CString(const CString& b)
		: m_ptr(b.m_ptr)
	{
	}

	/// Copy.
	constexpr CString& operator=(const CString& b)
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
	template<typename T>
	const Char& operator[](T pos) const
	{
		checkInit();
		ANKI_ASSERT(pos >= 0 && U32(pos) <= getLength());
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

	/// Find a substring of this string.
	/// @param[in] cstr The substring to search.
	/// @param position Position of the first character in the string to be considered in the search.
	/// @return A valid position if the string is found or kNpos if not found.
	PtrSize find(const CString& cstr, PtrSize position = 0) const
	{
		checkInit();
		ANKI_ASSERT(position < getLength());
		const Char* out = std::strstr(m_ptr + position, &cstr[0]);
		return (out == nullptr) ? kNpos : PtrSize(out - m_ptr);
	}

	/// Convert to F16.
	Error toNumber(F16& out) const;

	/// Convert to F32.
	Error toNumber(F32& out) const;

	/// Convert to F64.
	Error toNumber(F64& out) const;

	/// Convert to I8.
	Error toNumber(I8& out) const;

	/// Convert to U8.
	Error toNumber(U8& out) const;

	/// Convert to I16.
	Error toNumber(I16& out) const;

	/// Convert to U16.
	Error toNumber(U16& out) const;

	/// Convert to I32.
	Error toNumber(I32& out) const;

	/// Convert to U32.
	Error toNumber(U32& out) const;

	/// Convert to I64.
	Error toNumber(I64& out) const;

	/// Convert to U64.
	Error toNumber(U64& out) const;

	/// Convert to Bool.
	Error toNumber(Bool& out) const;

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
template<typename TMemoryPool>
class BaseString
{
	template<typename>
	friend class BaseStringList;

public:
	using Char = char; ///< Character type
	using Iterator = Char*;
	using ConstIterator = const Char*;
	using MemoryPool = TMemoryPool;

	static constexpr PtrSize kNpos = kMaxPtrSize;

	/// Default ctor.
	BaseString(const MemoryPool& pool = MemoryPool())
		: m_data(pool)
	{
	}

	/// Copy ctor.
	BaseString(const BaseString& b)
	{
		*this = b;
	}

	/// Copy ctor.
	BaseString(CString str, const MemoryPool& pool = MemoryPool())
		: m_data(pool)
	{
		*this = str;
	}

	/// Copy ctor.
	BaseString(const Char* str, const MemoryPool& pool = MemoryPool())
		: BaseString(CString(str), pool)
	{
	}

	/// Move ctor.
	BaseString(BaseString&& b)
		: m_data(std::move(b.m_data))
	{
	}

	/// Initialize using a range. Copies the range of [first, last)
	BaseString(ConstIterator first, ConstIterator last, const MemoryPool& pool = MemoryPool())
		: m_data(pool)
	{
		ANKI_ASSERT(first != nullptr && last != nullptr);
		for(ConstIterator it = first; it < last; ++it)
		{
			ANKI_ASSERT(*it != '\0');
		}

		const PtrSize length = last - first;
		m_data.resize(length + 1);

		memcpy(&m_data[0], first, length);
		m_data[length] = '\0';
	}

	/// Initialize using a character.
	BaseString(Char c, PtrSize length, const MemoryPool& pool = MemoryPool())
		: m_data(pool)
	{
		ANKI_ASSERT(c != '\0');
		m_data.resize(length + 1);
		memset(&m_data[0], c, length);
		m_data[length] = '\0';
	}

	~BaseString() = default;

	/// Move.
	BaseString& operator=(BaseString&& b)
	{
		m_data = std::move(b.m_data);
		return *this;
	}

	/// Copy.
	BaseString& operator=(const BaseString& b)
	{
		m_data = b.m_data;
		return *this;
	}

	/// Copy.
	BaseString& operator=(const Char* b)
	{
		*this = CString(b);
		return *this;
	}

	/// Copy.
	BaseString& operator=(CString str)
	{
		const U32 len = str.getLength();
		if(len > 0)
		{
			m_data.resize(len + 1);
			memcpy(&m_data[0], &str[0], sizeof(Char) * (len + 1));
		}
		else
		{
			m_data.destroy();
		}
		return *this;
	}

	/// Return char at the specified position.
	template<typename TInt>
	const Char& operator[](TInt pos) const
	{
		return m_data[pos];
	}

	/// Return char at the specified position as a modifiable reference.
	template<typename TInt>
	Char& operator[](TInt pos)
	{
		return m_data[pos];
	}

	explicit operator Bool() const
	{
		return !isEmpty();
	}

	Bool operator==(CString b) const
	{
		return toCString() == b;
	}

	Bool operator!=(CString b) const
	{
		return !(*this == b);
	}

	Bool operator<(CString b) const
	{
		return toCString() < b;
	}

	Bool operator<=(CString b) const
	{
		return toCString() < b;
	}

	Bool operator>(CString b) const
	{
		return toCString() > b;
	}

	Bool operator>=(CString b) const
	{
		return toCString() >= b;
	}

	/// Append another string to this one.
	BaseString& operator+=(CString b)
	{
		if(!b.isEmpty())
		{
			appendInternal(&b[0], b.getLength());
		}
		return *this;
	}

	operator CString() const
	{
		return toCString();
	}

	/// Get a C string.
	const Char* cstr() const
	{
		ANKI_ASSERT(!isEmpty());
		return &m_data[0];
	}

	/// Append using a range. Copies the range of [first, oneAfterLast)
	BaseString& append(ConstIterator first, ConstIterator oneAfterLast)
	{
		const PtrSize len = oneAfterLast - first;
		appendInternal(first, len);
		return *this;
	}

	/// Create formated string.
	ANKI_CHECK_FORMAT(1, 2)
	BaseString& sprintf(const Char* fmt, ...)
	{
		ANKI_ASSERT(fmt);
		va_list args;
		va_start(args, fmt);
		sprintfInternal(fmt, args);
		va_end(args);
		return *this;
	}

	/// Destroy the string.
	void destroy()
	{
		m_data.destroy();
	}

	Iterator getBegin()
	{
		ANKI_ASSERT(!isEmpty());
		return &m_data[0];
	}

	ConstIterator getBegin() const
	{
		ANKI_ASSERT(!isEmpty());
		return &m_data[0];
	}

	Iterator getEnd()
	{
		ANKI_ASSERT(!isEmpty());
		return &m_data[m_data.getSize() - 1];
	}

	ConstIterator getEnd() const
	{
		ANKI_ASSERT(!isEmpty());
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
	CString toCString() const
	{
		return (!isEmpty()) ? CString(&m_data[0]) : CString();
	}

	/// Return true if it's empty.
	Bool isEmpty() const
	{
		return m_data.isEmpty();
	}

	/// Find a substring of this string.
	/// @param[in] cstr The substring to search.
	/// @param position Position of the first character in the string to be considered in the search.
	/// @return A valid position if the string is found or kNpos if not found.
	PtrSize find(const CString& cstr, PtrSize position = 0) const
	{
		ANKI_ASSERT(!isEmpty());
		return toCString().find(cstr, position);
	}

	/// Find a substring of this string.
	/// @param[in] str The substring to search.
	/// @param position Position of the first character in the string to be considered in the search.
	/// @return A valid position if the string is found or kNpos if not found.
	PtrSize find(const BaseString& str, PtrSize position) const
	{
		ANKI_ASSERT(!isEmpty());
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	template<typename TNumber>
	void toString(TNumber number)
	{
		destroy();

		Array<Char, 512> buff;
		const I ret = std::snprintf(&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

		if(ret < 0 || ret > I(buff.getSize()))
		{
			ANKI_UTIL_LOGF("To small intermediate buffer");
		}
		else
		{
			*this = CString(&buff[0]);
		}
	}

	/// Convert to F16.
	Error toNumber(F16& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to F32.
	Error toNumber(F32& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to F64.
	Error toNumber(F64& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I8.
	Error toNumber(I8& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U8.
	Error toNumber(U8& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I16.
	Error toNumber(I16& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U16.
	Error toNumber(U16& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I32.
	Error toNumber(I32& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U32.
	Error toNumber(U32& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to I64.
	Error toNumber(I64& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to U64.
	Error toNumber(U64& out) const
	{
		return toCString().toNumber(out);
	}

	/// Convert to Bool.
	Error toNumber(Bool& out) const
	{
		return toCString().toNumber(out);
	}

	/// Compute the hash.
	U64 computeHash() const
	{
		ANKI_ASSERT(!isEmpty());
		return anki::computeHash(&m_data[0], m_data.getSize());
	}

	/// Replace all occurrences of "from" with "to".
	BaseString& replaceAll(CString from, CString to)
	{
		BaseString tmp(toCString(), getMemoryPool());
		const PtrSize fromLen = from.getLength();
		const PtrSize toLen = to.getLength();

		PtrSize pos = kNpos;
		while((pos = tmp.find(from)) != kNpos)
		{
			BaseString tmp2(getMemoryPool());
			if(pos > 0)
			{
				tmp2.append(tmp.getBegin(), tmp.getBegin() + pos);
			}

			if(toLen > 0)
			{
				tmp2.append(to.getBegin(), to.getBegin() + toLen);
			}

			if(pos + fromLen < tmp.getLength())
			{
				tmp2.append(tmp.getBegin() + pos + fromLen, tmp.getEnd());
			}

			tmp.destroy();
			tmp = std::move(tmp2);
		}

		destroy();
		*this = std::move(tmp);
		return *this;
	}

	/// @brief  Execute a functor for all characters of the string.
	template<typename TFunc>
	BaseString& transform(TFunc func)
	{
		U i = 0;
		while(i < m_data.getSize() && m_data[i] != '\0')
		{
			func(m_data[i]);
			++i;
		}
		return *this;
	}

	BaseString& toLower()
	{
		return transform([](Char& c) {
			c = Char(tolower(c));
		});
	}

	TMemoryPool& getMemoryPool()
	{
		return m_data.getMemoryPool();
	}

private:
	DynamicArray<Char, TMemoryPool, PtrSize> m_data;

	/// Append to this string.
	void appendInternal(const Char* str, PtrSize strLen)
	{
		ANKI_ASSERT(str != nullptr);
		ANKI_ASSERT(strLen > 0);
		ANKI_ASSERT(strLen == strlen(str));

		auto size = m_data.getSize();

		// Fix empty string
		if(size == 0)
		{
			size = 1;
		}

		m_data.resize(size + strLen);
		memcpy(&m_data[size - 1], str, sizeof(Char) * strLen);
		m_data[m_data.getSize() - 1] = '\0';
	}

	/// Internal don't use it.
	void sprintfInternal(const Char* fmt, va_list& args)
	{
		Array<Char, 512> buffer;

		va_list args2;
		va_copy(args2, args); // vsnprintf will alter "args". Copy it case we need to call vsnprintf in the else bellow

		I len = std::vsnprintf(&buffer[0], sizeof(buffer), fmt, args);

		if(len < 0)
		{
			ANKI_UTIL_LOGF("vsnprintf() failed");
		}
		else if(PtrSize(len) >= sizeof(buffer))
		{
			I size = len + 1;
			m_data.resize(size);

			len = std::vsnprintf(&m_data[0], size, fmt, args2);

			ANKI_ASSERT((len + 1) == size);
		}
		else
		{
			// buffer was enough
			*this = CString(&buffer[0]);
		}

		va_end(args2);
	}
};

using String = BaseString<SingletonMemoryPoolWrapper<DefaultMemoryPool>>;

#define ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, op, ...) \
	__VA_ARGS__ \
	inline Bool operator op(TypeA a, TypeB b) \
	{ \
		return CString(a) op CString(b); \
	}

#define ANKI_STRING_COMPARE_OPS(TypeA, TypeB, ...) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, ==, __VA_ARGS__) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, !=, __VA_ARGS__) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, <, __VA_ARGS__) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, <=, __VA_ARGS__) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, >, __VA_ARGS__) \
	ANKI_STRING_COMPARE_OPERATOR(TypeA, TypeB, >=, __VA_ARGS__)

ANKI_STRING_COMPARE_OPS(const Char*, CString, )
ANKI_STRING_COMPARE_OPS(const Char*, const BaseString<TMemPool>&, template<typename TMemPool>)
ANKI_STRING_COMPARE_OPS(const BaseString<TMemPool>&, const Char*, template<typename TMemPool>)
ANKI_STRING_COMPARE_OPS(CString, const BaseString<TMemPool>&, template<typename TMemPool>)
ANKI_STRING_COMPARE_OPS(const BaseString<TMemPool>&, const BaseString<TMemPool>&, template<typename TMemPool>)

#undef ANKI_STRING_COMPARE_OPERATOR
#undef ANKI_STRING_COMPARE_OPS
/// @}

} // end namespace anki
