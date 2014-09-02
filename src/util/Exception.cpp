// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Exception.h"
#include "anki/util/Array.h"
#include "anki/util/Memory.h"
#include <cstring>
#include <cstdarg>

// Instead of throwing abort. Its easier to debug
#define ANKI_ABORT_ON_THROW 0

namespace anki {

//==============================================================================
Exception::Exception(const char* file, I line, const char* func, 
	const char* fmt, ...) noexcept
{
	Array<char, 1024> buffer; 
	char* out = &buffer[0];
	va_list args;

	va_start(args, fmt);
	I len = std::vsnprintf(&buffer[0], sizeof(buffer), fmt, args);
	va_end(args);

	if(len < 0)
	{
		// Error in vsnprintf()
		std::strcpy(
			&buffer[0], "Error when throwing exception. vsnprintf() failed");
	}
	else if((PtrSize)len >= sizeof(buffer))
	{
		// The buffer is not big enough

		// Create a huge string
		I size = len + 1;
		out = reinterpret_cast<char*>(mallocAligned(size, 1));

		va_start(args, fmt);
		len = std::vsnprintf(out, size, fmt, args);
		(void)len;
		va_end(args);

		ANKI_ASSERT(len < size);
	}

	m_err = synthErr(out, file, line, func);

	// Delete the allocated memory
	if(out != &buffer[0])
	{
		freeAligned(out);
	}

#if ANKI_ABORT_ON_THROW == 1
	std::cerr << m_err << std::endl;
	abort();
#endif
}

//==============================================================================
Exception::Exception(const Exception& e) noexcept
{
	ANKI_ASSERT(e.m_err);

	m_err = reinterpret_cast<char*>(mallocAligned(std::strlen(e.m_err) + 1, 1));
	std::strcpy(m_err, e.m_err);

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
	U len = 
		std::strlen(error) + std::strlen(file) + 5 + std::strlen(func) + 5 + 10;

	char* out = reinterpret_cast<char*>(mallocAligned(len + 1, 1));
	
	I olen = std::snprintf(out, len + 1, "(%s:%d %s) %s", file, 
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

	U len = std::strlen(filling);
	len += std::strlen(m_err);
	len += std::strlen(e.what());

	out.m_err = reinterpret_cast<char*>(mallocAligned(len + 1, 1));
	I olen = std::snprintf(
		out.m_err, len + 1, "%s%s%s", m_err, filling, e.what());

	ANKI_ASSERT(olen >= 0 && (U)olen <= len);
	(void)olen;

	return out;
}

} // end namespace anki
