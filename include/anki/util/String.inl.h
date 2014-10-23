// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

//==============================================================================
template<typename TAlloc>
Error StringBase<TAlloc>::create(Allocator alloc, const CStringType& cstr)
{
	auto size = cstr.getLength() + 1;
	Error err = m_data.create(alloc, size);

	if(!err)
	{
		std::memcpy(&m_data[0], cstr.get(), sizeof(Char) * size);
	}

	return err;
}

//==============================================================================
template<typename TAlloc>
Error StringBase<TAlloc>::create(
	Allocator alloc, ConstIterator first, ConstIterator last)
{
	ANKI_ASSERT(first != 0 && last != 0);
	auto length = last - first;
	Error err = m_data.create(alloc, length + 1);

	if(!err)
	{
		std::memcpy(&m_data[0], first, length);
		m_data[length] = '\0';
	}

	return err;
}

//==============================================================================
template<typename TAlloc>
Error StringBase<TAlloc>::create(Allocator alloc, Char c, PtrSize length)
{
	ANKI_ASSERT(c != '\0');
	Error err = m_data.create(alloc, length + 1);

	if(!err)
	{
		std::memset(&m_data[0], c, length);
		m_data[length] = '\0';
	}

	return err;
}

//==============================================================================
template<typename TAlloc>
Error StringBase<TAlloc>::appendInternal(
	Allocator alloc, const Char* str, PtrSize strSize)
{
	ANKI_ASSERT(str != nullptr);
	ANKI_ASSERT(strSize > 1);

	auto size = m_data.getSize();

	// Fix empty string
	if(size == 0)
	{
		size = 1;
	}

	DArray<Char, Allocator> newData;	
	Error err = newData.create(alloc, size + strSize - 1);

	if(!err)
	{
		std::memcpy(&newData[0], &m_data[0], sizeof(Char) * size);
		std::memcpy(&newData[size - 1], str, sizeof(Char) * strSize);

		m_data.destroy(alloc);
		m_data = std::move(newData);
	}

	return err;
}

//==============================================================================
template<typename TAlloc>
Error StringBase<TAlloc>::sprintf(Allocator alloc, CString fmt, ...)
{
	Error err = ErrorCode::NONE;
	Array<Char, 512> buffer;
	va_list args;

	va_start(args, fmt);
	I len = std::vsnprintf(&buffer[0], sizeof(buffer), &fmt[0], args);
	va_end(args);

	if(len < 0)
	{
		ANKI_LOGE("vsnprintf() failed");
		err = ErrorCode::FUNCTION_FAILED;
	}
	else if(static_cast<PtrSize>(len) >= sizeof(buffer))
	{
		I size = len + 1;
		err = m_data.create(alloc, size);

		if(!err)
		{
			va_start(args, fmt);
			len = std::vsnprintf(&m_data[0], size, &fmt[0], args);
			va_end(args);

			(void)len;
			ANKI_ASSERT((len + 1) == size);
		}
	}
	else
	{
		// buffer was enough
		err = create(alloc, CString(&buffer[0]));
	}

	return err;
}

//==============================================================================
template<typename TAlloc>
template<typename TNumber>
Error StringBase<TAlloc>::toString(Allocator alloc, TNumber number, Self& out)
{
	Error err = ErrorCode::NONE;

	destroy(alloc);

	Array<Char, 512> buff;
	I ret = std::snprintf(
		&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

	if(ret < 0 || ret > static_cast<I>(buff.size()))
	{
		ANKI_LOGE("To small intermediate buffer");
		err = ErrorCode::FUNCTION_FAILED;
	}
	else
	{
		err = create(alloc, &buff[0]);
	}

	return err;
}

} // end namespace anki

