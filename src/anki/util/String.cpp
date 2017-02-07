// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

Error CString::toF64(F64& out) const
{
	checkInit();
	Error err = ErrorCode::NONE;
	out = std::strtod(m_ptr, nullptr);

	if(out == HUGE_VAL)
	{
		ANKI_UTIL_LOGE("Conversion failed");
		err = ErrorCode::USER_DATA;
	}

	return err;
}

Error CString::toF32(F32& out) const
{
	F64 d;
	Error err = toF64(d);
	if(!err)
	{
		out = d;
	}

	return err;
}

Error CString::toI64(I64& out) const
{
	checkInit();
	Error err = ErrorCode::NONE;
	out = std::strtoll(m_ptr, nullptr, 10);

	if(out == LLONG_MAX || out == LLONG_MIN)
	{
		ANKI_UTIL_LOGE("Conversion failed");
		err = ErrorCode::USER_DATA;
	}

	return err;
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

	std::memset(&m_data[0], c, length);
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
