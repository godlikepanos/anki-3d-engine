// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
class String
{
public:
	using Char = char; ///< Character type
	using CStringType = CString;
	using Iterator = Char*;
	using ConstIterator = const Char*;

	static constexpr PtrSize kNpos = kMaxPtrSize;

	/// Default constructor.
	String()
	{
	}

	/// Move constructor.
	String(String&& b)
	{
		*this = std::move(b);
	}

	template<typename TMemPool>
	String(BaseStringRaii<TMemPool>&& b)
	{
		*this = std::move(b);
	}

	template<typename TMemPool>
	String(TMemPool& pool, CString str)
	{
		create(pool, str);
	}

	String(const String&) = delete; // Non-copyable

	/// Requires manual destruction.
	~String()
	{
	}

	String& operator=(const String&) = delete; // Non-copyable

	/// Move one string to this one.
	String& operator=(String&& b)
	{
		move(b);
		return *this;
	}

	/// Move a BaseStringRaii to this one.
	template<typename TMemPool>
	String& operator=(BaseStringRaii<TMemPool>&& b)
	{
		m_data = std::move(b.m_data);
		return *this;
	}

	/// Return char at the specified position.
	template<typename TInt>
	const Char& operator[](TInt pos) const
	{
		checkInit();
		return m_data[pos];
	}

	/// Return char at the specified position as a modifiable reference.
	template<typename TInt>
	Char& operator[](TInt pos)
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
	template<typename TMemPool>
	void create(TMemPool& pool, const CStringType& cstr)
	{
		const U32 size = cstr.getLength() + 1;
		m_data.create(pool, size);
		memcpy(&m_data[0], &cstr[0], sizeof(Char) * size);
	}

	/// Initialize using a range. Copies the range of [first, last)
	template<typename TMemPool>
	void create(TMemPool& pool, ConstIterator first, ConstIterator last)
	{
		ANKI_ASSERT(first != 0 && last != 0);
		auto length = last - first;
		m_data.create(pool, length + 1);

		memcpy(&m_data[0], first, length);
		m_data[length] = '\0';
	}

	/// Initialize using a character.
	template<typename TMemPool>
	void create(TMemPool& pool, Char c, PtrSize length)
	{
		ANKI_ASSERT(c != '\0');
		m_data.create(pool, length + 1);
		memset(&m_data[0], c, length);
		m_data[length] = '\0';
	}

	/// Copy one string to this one.
	template<typename TMemPool>
	void create(TMemPool& pool, const String& b)
	{
		create(pool, b.toCString());
	}

	/// Append another string to this one.
	template<typename TMemPool>
	String& append(TMemPool& pool, const String& b)
	{
		if(!b.isEmpty())
		{
			appendInternal(pool, &b.m_data[0], b.m_data.getSize() - 1);
		}
		return *this;
	}

	/// Append a const string to this one.
	template<typename TMemPool>
	String& append(TMemPool& pool, const CStringType& cstr)
	{
		if(!cstr.isEmpty())
		{
			appendInternal(pool, cstr.cstr(), cstr.getLength());
		}
		return *this;
	}

	/// Append using a range. Copies the range of [first, oneAfterLast)
	template<typename TMemPool>
	String& append(TMemPool& pool, ConstIterator first, ConstIterator oneAfterLast)
	{
		const PtrSize len = oneAfterLast - first;
		appendInternal(pool, first, len);
		return *this;
	}

	/// Create formated string.
	template<typename TMemPool>
	ANKI_CHECK_FORMAT(2, 3)
	String& sprintf(TMemPool& pool, const Char* fmt, ...)
	{
		ANKI_ASSERT(fmt);
		va_list args;
		va_start(args, fmt);
		sprintfInternal(pool, fmt, args);
		va_end(args);
		return *this;
	}

	/// Destroy the string.
	template<typename TMemPool>
	void destroy(TMemPool& pool)
	{
		m_data.destroy(pool);
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
	/// @return A valid position if the string is found or kNpos if not found.
	PtrSize find(const CStringType& cstr, PtrSize position = 0) const
	{
		checkInit();
		return toCString().find(cstr, position);
	}

	/// Find a substring of this string.
	/// @param[in] str The substring to search.
	/// @param position Position of the first character in the string to be considered in the search.
	/// @return A valid position if the string is found or kNpos if not found.
	PtrSize find(const String& str, PtrSize position) const
	{
		str.checkInit();
		return find(str.toCString(), position);
	}

	/// Convert a number to a string.
	template<typename TMemPool, typename TNumber>
	void toString(TMemPool& pool, TNumber number)
	{
		destroy(pool);

		Array<Char, 512> buff;
		const I ret = std::snprintf(&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

		if(ret < 0 || ret > static_cast<I>(buff.getSize()))
		{
			ANKI_UTIL_LOGF("To small intermediate buffer");
		}
		else
		{
			create(pool, &buff[0]);
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
		checkInit();
		return anki::computeHash(&m_data[0], m_data.getSize());
	}

	/// Replace all occurrences of "from" with "to".
	template<typename TMemPool>
	String& replaceAll(TMemPool& pool, CString from, CString to)
	{
		String tmp(pool, toCString());
		const PtrSize fromLen = from.getLength();
		const PtrSize toLen = to.getLength();

		PtrSize pos = kNpos;
		while((pos = tmp.find(from)) != kNpos)
		{
			String tmp2;
			if(pos > 0)
			{
				tmp2.create(pool, tmp.getBegin(), tmp.getBegin() + pos);
			}

			if(toLen > 0)
			{
				tmp2.append(pool, to.getBegin(), to.getBegin() + toLen);
			}

			if(pos + fromLen < tmp.getLength())
			{
				tmp2.append(pool, tmp.getBegin() + pos + fromLen, tmp.getEnd());
			}

			tmp.destroy(pool);
			tmp = std::move(tmp2);
		}

		destroy(pool);
		*this = std::move(tmp);
		return *this;
	}

	/// @brief  Execute a functor for all characters of the string.
	template<typename TFunc>
	String& transform(TFunc func)
	{
		U i = 0;
		while(i < m_data.getSize() && m_data[i] != '\0')
		{
			func(m_data[i]);
			++i;
		}
		return *this;
	}

	String& toLower()
	{
		return transform([](Char& c) {
			c = Char(tolower(c));
		});
	}

	/// Internal don't use it.
	template<typename TMemPool>
	void sprintfInternal(TMemPool& pool, const Char* fmt, va_list& args)
	{
		Array<Char, 512> buffer;

		va_list args2;
		va_copy(args2, args); // vsnprintf will alter "args". Copy it case we need to call vsnprintf in the else bellow

		I len = std::vsnprintf(&buffer[0], sizeof(buffer), fmt, args);

		if(len < 0)
		{
			ANKI_UTIL_LOGF("vsnprintf() failed");
		}
		else if(static_cast<PtrSize>(len) >= sizeof(buffer))
		{
			I size = len + 1;
			m_data.create(pool, size);

			len = std::vsnprintf(&m_data[0], size, fmt, args2);

			ANKI_ASSERT((len + 1) == size);
		}
		else
		{
			// buffer was enough
			create(pool, CString(&buffer[0]));
		}

		va_end(args2);
	}

protected:
	DynamicArray<Char, PtrSize> m_data;

	void checkInit() const
	{
		ANKI_ASSERT(m_data.getSize() > 0);
	}

	/// Append to this string.
	template<typename TMemPool>
	void appendInternal(TMemPool& pool, const Char* str, PtrSize strLen)
	{
		ANKI_ASSERT(str != nullptr);
		ANKI_ASSERT(strLen > 0);

		auto size = m_data.getSize();

		// Fix empty string
		if(size == 0)
		{
			size = 1;
		}

		DynamicArray<Char, PtrSize> newData;
		newData.create(pool, size + strLen);

		if(!m_data.isEmpty())
		{
			memcpy(&newData[0], &m_data[0], sizeof(Char) * size);
		}

		memcpy(&newData[size - 1], str, sizeof(Char) * strLen);

		newData[newData.getSize() - 1] = '\0';

		m_data.destroy(pool);
		m_data = std::move(newData);
	}

	void move(String& b)
	{
		ANKI_ASSERT(this != &b);
		m_data = std::move(b.m_data);
	}
};

/// String with automatic cleanup.
template<typename TMemPool = MemoryPoolPtrWrapper<BaseMemoryPool>>
class BaseStringRaii : public String
{
public:
	using Base = String;
	using MemoryPool = TMemPool;

	/// Create with pool.
	BaseStringRaii(const MemoryPool& pool)
		: Base()
		, m_pool(pool)
	{
	}

	/// Copy construcor.
	BaseStringRaii(const BaseStringRaii& b)
		: Base()
		, m_pool(b.m_pool)
	{
		if(!b.isEmpty())
		{
			create(b.m_data.getBegin(), b.m_data.getEnd());
		}
	}

	/// Move constructor.
	BaseStringRaii(BaseStringRaii&& b)
		: Base()
	{
		move(b);
	}

	/// Create with memory pool and data.
	BaseStringRaii(const MemoryPool& pool, const CStringType& cstr)
		: Base()
		, m_pool(pool)
	{
		create(cstr);
	}

	/// Automatic destruction.
	~BaseStringRaii()
	{
		Base::destroy(m_pool);
	}

	/// Copy operator.
	BaseStringRaii& operator=(const BaseStringRaii& b)
	{
		destroy();
		m_pool = b.m_pool;
		if(!b.isEmpty())
		{
			create(b.m_data.getBegin(), b.m_data.getEnd());
		}
		return *this;
	}

	/// Copy from string.
	BaseStringRaii& operator=(const CString& b)
	{
		destroy();
		if(!b.isEmpty())
		{
			create(b.getBegin(), b.getEnd());
		}
		return *this;
	}

	/// Move one string to this one.
	BaseStringRaii& operator=(BaseStringRaii&& b)
	{
		destroy();
		move(b);
		return *this;
	}

	const MemoryPool& getMemoryPool() const
	{
		return m_pool;
	}

	MemoryPool& getMemoryPool()
	{
		return m_pool;
	}

	/// Initialize using a const string.
	void create(const CStringType& cstr)
	{
		Base::create(m_pool, cstr);
	}

	/// Initialize using a range. Copies the range of [first, last)
	void create(ConstIterator first, ConstIterator last)
	{
		Base::create(m_pool, first, last);
	}

	/// Initialize using a character.
	void create(Char c, PtrSize length)
	{
		Base::create(m_pool, c, length);
	}

	/// Copy one string to this one.
	void create(const String& b)
	{
		Base::create(m_pool, b.toCString());
	}

	/// Destroy the string.
	void destroy()
	{
		Base::destroy(m_pool);
	}

	/// Append another string to this one.
	BaseStringRaii& append(const String& b)
	{
		Base::append(m_pool, b);
		return *this;
	}

	/// Append a const string to this one.
	BaseStringRaii& append(const CStringType& cstr)
	{
		Base::append(m_pool, cstr);
		return *this;
	}

	/// Create formated string.
	ANKI_CHECK_FORMAT(1, 2)
	BaseStringRaii& sprintf(const Char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		Base::sprintfInternal(m_pool, fmt, args);
		va_end(args);
		return *this;
	}

	/// Convert a number to a string.
	template<typename TNumber>
	void toString(TNumber number)
	{
		Base::toString(m_pool, number);
	}

	/// Replace all occurrences of "from" with "to".
	BaseStringRaii& replaceAll(CString from, CString to)
	{
		Base::replaceAll(m_pool, from, to);
		return *this;
	}

private:
	MemoryPool m_pool;

	void move(BaseStringRaii& b)
	{
		Base::move(b);
		m_pool = std::move(b.m_pool);
	}
};

using StringRaii = BaseStringRaii<>;

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
ANKI_STRING_COMPARE_OPS(const Char*, const String&, )
ANKI_STRING_COMPARE_OPS(const Char*, const BaseStringRaii<TMemPool>&, template<typename TMemPool>)

ANKI_STRING_COMPARE_OPS(CString, const Char*, )
ANKI_STRING_COMPARE_OPS(CString, const String&, )
ANKI_STRING_COMPARE_OPS(CString, const BaseStringRaii<TMemPool>&, template<typename TMemPool>)

ANKI_STRING_COMPARE_OPS(const String&, const Char*, )
ANKI_STRING_COMPARE_OPS(const String&, CString, )
ANKI_STRING_COMPARE_OPS(const String&, const BaseStringRaii<TMemPool>&, template<typename TMemPool>)

ANKI_STRING_COMPARE_OPS(const BaseStringRaii<TMemPool>&, const Char*, template<typename TMemPool>)
ANKI_STRING_COMPARE_OPS(const BaseStringRaii<TMemPool>&, CString, template<typename TMemPool>)
ANKI_STRING_COMPARE_OPS(const BaseStringRaii<TMemPool>&, const String&, template<typename TMemPool>)
ANKI_STRING_COMPARE_OPS(const BaseStringRaii<TMemPool>&, const BaseStringRaii<TMemPool>&, template<typename TMemPool>)

#undef ANKI_STRING_COMPARE_OPERATOR
#undef ANKI_STRING_COMPARE_OPS
/// @}

} // end namespace anki
