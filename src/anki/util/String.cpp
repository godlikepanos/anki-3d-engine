// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/String.h>
#include <cmath> // For HUGE_VAL
#include <climits> // For LLONG_MAX
#include <cstdarg> // For var args
#include <cstdlib> // For stdtod and strtol

namespace anki
{

Error CString::toNumber(F64& out) const
{
	checkInit();
	errno = 0;
	out = std::strtod(m_ptr, nullptr);

	if(errno)
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error CString::toNumber(F32& out) const
{
	F64 d;
	ANKI_CHECK(toNumber(d));
	out = d;
	return Error::NONE;
}

Error CString::toNumber(I8& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < MIN_I8 || i64 > MAX_I8)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range");
		return Error::USER_DATA;
	}

	out = I8(i64);
	return Error::NONE;
}

Error CString::toNumber(I64& out) const
{
	checkInit();
	errno = 0;
	static_assert(sizeof(long long) == sizeof(I64), "See file");
	out = std::strtoll(m_ptr, nullptr, 10);

	if(errno)
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error CString::toNumber(I32& out) const
{
	checkInit();
	errno = 0;
	long long i = std::strtoll(m_ptr, nullptr, 10);

	if(errno || i < MIN_I32 || i > MAX_I32)
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed");
		return Error::USER_DATA;
	}

	out = I32(i);

	return Error::NONE;
}

Error CString::toNumber(U64& out) const
{
	checkInit();
	errno = 0;
	static_assert(sizeof(unsigned long long) == sizeof(U64), "See file");
	out = std::strtoull(m_ptr, nullptr, 10);

	if(errno)
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error CString::toNumber(U32& out) const
{
	checkInit();
	errno = 0;
	unsigned long long i = std::strtoull(m_ptr, nullptr, 10);

	if(errno || i > MAX_U32)
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed");
		return Error::USER_DATA;
	}

	out = U32(i);
	return Error::NONE;
}

Error CString::toNumber(U8& out) const
{
	U64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 > MAX_U8)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range");
		return Error::USER_DATA;
	}

	out = U8(i64);
	return Error::NONE;
}

Error CString::toNumber(Bool& out) const
{
	I32 i;
	ANKI_CHECK(toNumber(i));
	out = i != 0;
	return Error::NONE;
}

String& String::operator=(StringAuto&& b)
{
	m_data = std::move(b.m_data);
	return *this;
}

void String::create(Allocator alloc, const CStringType& cstr)
{
	auto len = cstr.getLength();
	if(len > 0)
	{
		auto size = len + 1;
		m_data.create(alloc, size);
		std::memcpy(&m_data[0], &cstr[0], sizeof(Char) * size);
	}
}

void String::create(Allocator alloc, ConstIterator first, ConstIterator last)
{
	ANKI_ASSERT(first != 0 && last != 0);
	auto length = last - first;
	m_data.create(alloc, length + 1);

	std::memcpy(&m_data[0], first, length);
	m_data[length] = '\0';
}

void String::create(Allocator alloc, Char c, PtrSize length)
{
	ANKI_ASSERT(c != '\0');
	m_data.create(alloc, length + 1);

	memset(&m_data[0], c, length);
	m_data[length] = '\0';
}

void String::appendInternal(Allocator alloc, const Char* str, PtrSize strSize)
{
	ANKI_ASSERT(str != nullptr);
	ANKI_ASSERT(strSize > 1);

	auto size = m_data.getSize();

	// Fix empty string
	if(size == 0)
	{
		size = 1;
	}

	DynamicArray<Char> newData;
	newData.create(alloc, size + strSize - 1);

	if(!m_data.isEmpty())
	{
		std::memcpy(&newData[0], &m_data[0], sizeof(Char) * size);
	}

	std::memcpy(&newData[size - 1], str, sizeof(Char) * strSize);

	m_data.destroy(alloc);
	m_data = std::move(newData);
}

void String::sprintf(Allocator alloc, CString fmt, ...)
{
	Array<Char, 512> buffer;
	va_list args;

	va_start(args, fmt);
	I len = std::vsnprintf(&buffer[0], sizeof(buffer), &fmt[0], args);
	va_end(args);

	if(len < 0)
	{
		ANKI_UTIL_LOGF("vsnprintf() failed");
	}
	else if(static_cast<PtrSize>(len) >= sizeof(buffer))
	{
		I size = len + 1;
		m_data.create(alloc, size);

		va_start(args, fmt);
		len = std::vsnprintf(&m_data[0], size, &fmt[0], args);
		va_end(args);

		(void)len;
		ANKI_ASSERT((len + 1) == size);
	}
	else
	{
		// buffer was enough
		create(alloc, CString(&buffer[0]));
	}
}

} // end namespace anki
