// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/StringList.h"
#include <cstring>

namespace anki {

//==============================================================================
template<typename TChar, typename TAlloc>
typename BasicStringList<TChar, TAlloc>::String 
	BasicStringList<TChar, TAlloc>::join(const Char* separator) const
{
	// Count the characters
	I sepLen = std::strlen(separator);
	I charCount = 0;
	for(const String& str : *this)
	{
		charCount += str.length() + sepLen;
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
template<typename TChar, typename TAlloc>
I BasicStringList<TChar, TAlloc>::getIndexOf(const Char* value) const
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
template<typename TChar, typename TAlloc>
BasicStringList<TChar, TAlloc> 
	BasicStringList<TChar, TAlloc>::splitString(
	const Char* s, 
	const Char separator,
	Allocator alloc)
{
	ANKI_ASSERT(s != nullptr);

	Self out(alloc);
	const Char* begin = s;
	const Char* end = begin;

	while(true)
	{
		if(*end == '\0')
		{
			if(begin < end)
			{
				out.push_back(String(begin, end, alloc));
			}

			break;
		}
		else if(*end == separator)
		{
			if(begin < end)
			{
				out.push_back(String(begin, end, alloc));
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
