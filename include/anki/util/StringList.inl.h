// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/StringList.h"
#include <cstring>

namespace anki {

//==============================================================================
template<typename TAlloc>
typename StringListBase<TAlloc>::String 
	StringListBase<TAlloc>::join(const CString& separator) const
{
	// Count the characters
	I sepLen = separator.getLength();
	I charCount = 0;
	for(const String& str : *this)
	{
		charCount += str.getLength() + sepLen;
	}

	charCount -= sepLen; // Remove last separator
	ANKI_ASSERT(charCount > 0);

	Allocator alloc = Base::get_allocator();
	String out(alloc);
	out.reserve(charCount + 1);

	typename Base::const_iterator it = Base::begin();
	for(; it != Base::end(); it++)
	{
		out += *it;
		if(it != Base::end() - 1)
		{
			out += separator;
		}
	}

	return out;
}

//==============================================================================
template<typename TAlloc>
I StringListBase<TAlloc>::getIndexOf(const CString& value) const
{
	U pos = 0;

	for(auto it = Base::begin(); it != Base::end(); ++it)
	{
		if(*it == value)
		{
			break;
		}
		++ pos;
	}

	return (pos == Base::size()) ? -1 : pos;
}

//==============================================================================
template<typename TAlloc>
StringListBase<TAlloc> 
	StringListBase<TAlloc>::splitString(
	const CString& s, 
	const Char separator,
	Allocator alloc)
{
	Self out(alloc);
	const Char* begin = &s[0];
	const Char* end = begin;

	while(true)
	{
		if(*end == '\0')
		{
			if(begin < end)
			{
				out.emplace_back(begin, end, alloc);
			}

			break;
		}
		else if(*end == separator)
		{
			if(begin < end)
			{
				out.emplace_back(begin, end, alloc);
				begin = end + 1;
			}
			else
			{
				++begin;
			}
		}

		++end;
	}

	return out;
}

} // end namespace anki
