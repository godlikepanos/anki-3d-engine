// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

//==============================================================================
template<typename TAllocator>
inline void String::create(TAllocator alloc, const CStringType& cstr)
{
	auto len = cstr.getLength();
	if(len > 0)
	{
		auto size = len + 1;
		m_data.create(alloc, size);
		std::memcpy(&m_data[0], &cstr[0], sizeof(Char) * size);
	}
}

//==============================================================================
template<typename TAllocator>
inline void String::create(
	TAllocator alloc, ConstIterator first, ConstIterator last)
{
	ANKI_ASSERT(first != 0 && last != 0);
	auto length = last - first;
	m_data.create(alloc, length + 1);

	std::memcpy(&m_data[0], first, length);
	m_data[length] = '\0';
}

//==============================================================================
template<typename TAllocator>
inline void String::create(TAllocator alloc, Char c, PtrSize length)
{
	ANKI_ASSERT(c != '\0');
	m_data.create(alloc, length + 1);

	std::memset(&m_data[0], c, length);
	m_data[length] = '\0';
}

//==============================================================================
template<typename TAllocator>
inline void String::appendInternal(
	TAllocator alloc, const Char* str, PtrSize strSize)
{
	ANKI_ASSERT(str != nullptr);
	ANKI_ASSERT(strSize > 1);

	auto size = m_data.getSize();

	// Fix empty string
	if(size == 0)
	{
		size = 1;
	}

	DArray<Char> newData;	
	newData.create(alloc, size + strSize - 1);

	if(!m_data.isEmpty())
	{
		std::memcpy(&newData[0], &m_data[0], sizeof(Char) * size);
	}

	std::memcpy(&newData[size - 1], str, sizeof(Char) * strSize);

	m_data.destroy(alloc);
	m_data = std::move(newData);
}

//==============================================================================
template<typename TAllocator>
inline void String::sprintf(TAllocator alloc, CString fmt, ...)
{
	Array<Char, 512> buffer;
	va_list args;

	va_start(args, fmt);
	I len = std::vsnprintf(&buffer[0], sizeof(buffer), &fmt[0], args);
	va_end(args);

	if(len < 0)
	{
		ANKI_LOGF("vsnprintf() failed");
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

//==============================================================================
template<typename TAllocator, typename TNumber>
inline void String::toString(TAllocator alloc, TNumber number)
{
	destroy(alloc);

	Array<Char, 512> buff;
	I ret = std::snprintf(
		&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

	if(ret < 0 || ret > static_cast<I>(buff.getSize()))
	{
		ANKI_LOGF("To small intermediate buffer");
	}
	else
	{
		create(alloc, &buff[0]);
	}
}

} // end namespace anki

