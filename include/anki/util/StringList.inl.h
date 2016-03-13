// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/StringList.h>
#include <cstring>

namespace anki
{

//==============================================================================
template<typename TAllocator>
inline void StringList::destroy(TAllocator alloc)
{
	auto it = Base::getBegin();
	auto endit = Base::getEnd();
	for(; it != endit; ++it)
	{
		(*it).destroy(alloc);
	}

	Base::destroy(alloc);
}

//==============================================================================
template<typename TAllocator>
inline void StringList::join(
	TAllocator alloc, const CString& separator, String& out) const
{
	if(Base::isEmpty())
	{
		return;
	}

	// Count the characters
	const I sepLen = separator.getLength();
	I charCount = 0;
	for(const String& str : *this)
	{
		charCount += str.getLength() + sepLen;
	}

	charCount -= sepLen; // Remove last separator
	ANKI_ASSERT(charCount > 0);

	// Allocate
	out.create(alloc, '?', charCount);

	// Append to output
	Char* to = &out[0];
	typename Base::ConstIterator it = Base::getBegin();
	for(; it != Base::getEnd(); it++)
	{
		const String& from = *it;
		std::memcpy(to, &from[0], from.getLength() * sizeof(Char));
		to += from.getLength();

		if(it != Base::end() - 1)
		{
			std::memcpy(to, &separator[0], sepLen * sizeof(Char));
			to += sepLen;
		}
	}
}

//==============================================================================
template<typename TAllocator>
inline void StringList::splitString(
	TAllocator alloc, const CString& s, const Char separator)
{
	ANKI_ASSERT(Base::isEmpty());

	const Char* begin = &s[0];
	const Char* end = begin;

	while(1)
	{
		if(*end == '\0')
		{
			if(begin < end)
			{
				Base::emplaceBack(alloc);

				String str;
				str.create(alloc, begin, end);
				Base::getBack() = std::move(str);
			}

			break;
		}
		else if(*end == separator)
		{
			if(begin < end)
			{
				Base::emplaceBack(alloc);

				String str;
				str.create(alloc, begin, end);

				Base::getBack() = std::move(str);
				begin = end + 1;
			}
			else
			{
				++begin;
			}
		}

		++end;
	}
}

//==============================================================================
template<typename TAllocator, typename... TArgs>
inline void StringList::pushBackSprintf(TAllocator alloc, const TArgs&... args)
{
	String str;
	str.sprintf(alloc, args...);

	Base::emplaceBack(alloc);
	Base::getBack() = std::move(str);
}

} // end namespace anki
