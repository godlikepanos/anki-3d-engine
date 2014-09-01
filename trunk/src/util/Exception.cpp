// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Exception.h"
#include "anki/util/Vector.h"
#include "anki/util/Memory.h"
#include <cstring>
#include <cstdarg>

// Instead of throwing abort. Its easier to debug
#define ANKI_ABORT_ON_THROW 0

namespace anki {

//==============================================================================
Exception::Exception(const CString& file, I line, const CString& func, 
	const CString& fmt, ...) noexcept
{
	Array<char, 1024> buff; 
	const char* out = &buffer[0];
	va_list args;

	va_start(args, fmt);
	I len = vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	if(len < 0)
	{
		// Error in vsnprintf()
		strcpy(buffer, "Error when throwing exception. vsnprintf() failed");
	}
	else if((PtrSize)len >= sizeof(buffer))
	{
		// The buffer is not big enough

		// Create a huge string
		I size = len + 1;
		out = mallocAligned(size, 1);

		va_start(args, fmt);
		len = vsnprintf(out, size, fmt, args);
		(void)len;
		va_end(args);

		ANKI_ASSERT(len < size);
	}

	m_err = synthErr(out, file, line, func);

#if ANKI_ABORT_ON_THROW == 1
	std::cerr << m_err << std::endl;
	abort();
#endif
}

//==============================================================================
Exception::Exception(const Exception& e) noexcept
{
	ANKI_ASSERT(e.m_err);

	m_err = (char*)mallocAligned(strlen(e.m_err) + 1, 1);
	strcpy(m_err, e.m_err);

#if ANKI_ABORT_ON_THROW == 1
	std::cerr << m_err << std::endl;
	abort();
#endif
}

//==============================================================================
Exception::~Exception() noexcept
{
	if(m_err)
	{
		freeAligned(m_err);
		m_err = nullptr;
	}
}

//==============================================================================
Exception& Exception::operator=(Exception&& b) noexcept
{
	if(m_err)
	{
		freeAligned(m_err);
	}

	m_err = b.m_err;
	b.m_err = nullptr;
	return *this;
}

//==============================================================================
char* Exception::synthErr(const char* error, const char* file,
	I line, const char* func) noexcept
{
	// The length of all strings plus some extra chars for the formating 
	// plus 10 to be safe
	U len = strlen(error) + strlen(file) + 5 + strlen(func) + 5 + 10;

	char* out = (char*)mallocAligned(len + 1, 1);
	
	I olen = snprintf(out, len + 1, "(%s:%d %s) %s", file, 
		static_cast<int>(line), func, error);
	
	ANKI_ASSERT(olen >= 0 && (U)olen <= len);
	(void)olen;

	return out;
}

//==============================================================================
Exception Exception::operator<<(const std::exception& e) const
{
	ANKI_ASSERT(m_err);
	ANKI_ASSERT(e.what());

	static const char* filling = "\nFrom: ";
	Exception out;

	U len = strlen(filling);
	len += strlen(m_err);
	len += strlen(e.what());

	out.m_err = (char*)mallocAligned(len + 1, 1);
	I olen = snprintf(out.m_err, len + 1, "%s%s%s", m_err, filling, e.what());
	ANKI_ASSERT(olen >= 0 && (U)olen <= len);
	(void)olen;

	return out;
}

} // end namespace anki
