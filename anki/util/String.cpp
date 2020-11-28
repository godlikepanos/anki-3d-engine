// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/String.h>
#include <anki/util/F16.h>
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
	char* endPtr;
	out = std::strtod(m_ptr, &endPtr);

	if(errno || endPtr != m_ptr + getLength())
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed: %s", m_ptr);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error CString::toNumber(F32& out) const
{
	F64 d;
	ANKI_CHECK(toNumber(d));
	out = F32(d);
	return Error::NONE;
}

Error CString::toNumber(F16& out) const
{
	F64 d;
	ANKI_CHECK(toNumber(d));
	out = F16(d);
	return Error::NONE;
}

Error CString::toNumber(I64& out) const
{
	checkInit();
	errno = 0;
	char* endPtr;
	static_assert(sizeof(long long) == sizeof(I64), "See file");
	out = std::strtoll(m_ptr, &endPtr, 10);

	if(errno || endPtr != m_ptr + getLength())
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed: %s", m_ptr);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error CString::toNumber(I8& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < MIN_I8 || i64 > MAX_I8)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::USER_DATA;
	}

	out = I8(i64);
	return Error::NONE;
}

Error CString::toNumber(I32& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < MIN_I32 || i64 > MAX_I32)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::USER_DATA;
	}

	out = I32(i64);
	return Error::NONE;
}

Error CString::toNumber(U64& out) const
{
	checkInit();
	errno = 0;
	char* endPtr;
	static_assert(sizeof(unsigned long long) == sizeof(U64), "See file");
	out = std::strtoull(m_ptr, &endPtr, 10);

	if(errno || endPtr != m_ptr + getLength())
	{
		errno = 0;
		ANKI_UTIL_LOGE("Conversion failed: %s", m_ptr);
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error CString::toNumber(U32& out) const
{
	U64 u64;
	ANKI_CHECK(toNumber(u64));

	if(u64 > MAX_U32)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::USER_DATA;
	}

	out = U32(u64);
	return Error::NONE;
}

Error CString::toNumber(U8& out) const
{
	U64 u64 = 0;
	ANKI_CHECK(toNumber(u64));

	if(u64 > MAX_U8)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::USER_DATA;
	}

	out = U8(u64);
	return Error::NONE;
}

Error CString::toNumber(I16& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < MIN_I16 || i64 > MAX_I16)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::USER_DATA;
	}

	out = I16(i64);
	return Error::NONE;
}

Error CString::toNumber(U16& out) const
{
	U64 u64;
	ANKI_CHECK(toNumber(u64));

	if(u64 > MAX_U16)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::USER_DATA;
	}

	out = U16(u64);
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
	auto size = len + 1;
	m_data.create(alloc, size);
	memcpy(&m_data[0], &cstr[0], sizeof(Char) * size);
}

void String::create(Allocator alloc, ConstIterator first, ConstIterator last)
{
	ANKI_ASSERT(first != 0 && last != 0);
	auto length = last - first;
	m_data.create(alloc, length + 1);

	memcpy(&m_data[0], first, length);
	m_data[length] = '\0';
}

void String::create(Allocator alloc, Char c, PtrSize length)
{
	ANKI_ASSERT(c != '\0');
	m_data.create(alloc, length + 1);

	memset(&m_data[0], c, length);
	m_data[length] = '\0';
}

void String::appendInternal(Allocator& alloc, const Char* str, PtrSize strLen)
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
	newData.create(alloc, size + strLen);

	if(!m_data.isEmpty())
	{
		memcpy(&newData[0], &m_data[0], sizeof(Char) * size);
	}

	memcpy(&newData[size - 1], str, sizeof(Char) * strLen);

	newData[newData.getSize() - 1] = '\0';

	m_data.destroy(alloc);
	m_data = std::move(newData);
}

String& String::sprintf(Allocator alloc, CString fmt, ...)
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

	return *this;
}

String& String::replaceAll(Allocator alloc, CString from, CString to)
{
	String tmp = {alloc, toCString()};
	const PtrSize fromLen = from.getLength();
	const PtrSize toLen = to.getLength();

	PtrSize pos = NPOS;
	while((pos = tmp.find(from)) != NPOS)
	{
		String tmp2;
		if(pos > 0)
		{
			tmp2.create(alloc, tmp.getBegin(), tmp.getBegin() + pos);
		}

		if(toLen > 0)
		{
			tmp2.append(alloc, to.getBegin(), to.getBegin() + toLen);
		}

		if(pos + fromLen < tmp.getLength())
		{
			tmp2.append(alloc, tmp.getBegin() + pos + fromLen, tmp.getEnd());
		}

		tmp.destroy(alloc);
		tmp = std::move(tmp2);
	}

	destroy(alloc);
	*this = std::move(tmp);
	return *this;
}

} // end namespace anki
