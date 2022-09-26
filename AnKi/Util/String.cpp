// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/String.h>
#include <AnKi/Util/F16.h>
#include <cmath> // For HUGE_VAL
#include <climits> // For LLONG_MAX
#include <cstdlib> // For stdtod and strtol

namespace anki {

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
		return Error::kUserData;
	}

	return Error::kNone;
}

Error CString::toNumber(F32& out) const
{
	F64 d;
	ANKI_CHECK(toNumber(d));
	out = F32(d);
	return Error::kNone;
}

Error CString::toNumber(F16& out) const
{
	F64 d;
	ANKI_CHECK(toNumber(d));
	out = F16(d);
	return Error::kNone;
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
		return Error::kUserData;
	}

	return Error::kNone;
}

Error CString::toNumber(I8& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < kMinI8 || i64 > kMaxI8)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::kUserData;
	}

	out = I8(i64);
	return Error::kNone;
}

Error CString::toNumber(I32& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < kMinI32 || i64 > kMaxI32)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::kUserData;
	}

	out = I32(i64);
	return Error::kNone;
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
		return Error::kUserData;
	}

	return Error::kNone;
}

Error CString::toNumber(U32& out) const
{
	U64 u64;
	ANKI_CHECK(toNumber(u64));

	if(u64 > kMaxU32)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::kUserData;
	}

	out = U32(u64);
	return Error::kNone;
}

Error CString::toNumber(U8& out) const
{
	U64 u64 = 0;
	ANKI_CHECK(toNumber(u64));

	if(u64 > kMaxU8)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::kUserData;
	}

	out = U8(u64);
	return Error::kNone;
}

Error CString::toNumber(I16& out) const
{
	I64 i64 = 0;
	ANKI_CHECK(toNumber(i64));

	if(i64 < kMinI16 || i64 > kMaxI16)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::kUserData;
	}

	out = I16(i64);
	return Error::kNone;
}

Error CString::toNumber(U16& out) const
{
	U64 u64;
	ANKI_CHECK(toNumber(u64));

	if(u64 > kMaxU16)
	{
		ANKI_UTIL_LOGE("Conversion failed. Our of range: %s", m_ptr);
		return Error::kUserData;
	}

	out = U16(u64);
	return Error::kNone;
}

Error CString::toNumber(Bool& out) const
{
	I32 i;
	ANKI_CHECK(toNumber(i));
	out = i != 0;
	return Error::kNone;
}

} // end namespace anki
