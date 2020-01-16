// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/StringList.h>

namespace anki
{

void StringList::destroy(Allocator alloc)
{
	auto it = Base::getBegin();
	auto endit = Base::getEnd();
	for(; it != endit; ++it)
	{
		(*it).destroy(alloc);
	}

	Base::destroy(alloc);
}

void StringList::join(Allocator alloc, const CString& separator, String& out) const
{
	StringAuto outl(alloc);
	join(separator, outl);
	out = std::move(outl);
}

void StringList::join(const CString& separator, StringAuto& out) const
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
	out.create('?', charCount);

	// Append to output
	Char* to = &out[0];
	typename Base::ConstIterator it = Base::getBegin();
	for(; it != Base::getEnd(); it++)
	{
		const String& from = *it;
		memcpy(to, &from[0], from.getLength() * sizeof(Char));
		to += from.getLength();

		if(it != Base::end() - 1)
		{
			memcpy(to, &separator[0], sepLen * sizeof(Char));
			to += sepLen;
		}
	}
}

void StringList::splitString(Allocator alloc, const CString& s, const Char separator, Bool keepEmpty)
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
				if(keepEmpty)
				{
					Base::emplaceBack(alloc);
				}

				++begin;
			}
		}

		++end;
	}
}

void StringList::sortAll(const Sort method)
{
	if(method == Sort::ASCENDING)
	{
		Base::sort([](const String& a, const String& b) { return a < b; });
	}
	else
	{
		ANKI_ASSERT(method == Sort::DESCENDING);
		Base::sort([](const String& a, const String& b) { return a < b; });
	}
}

I StringList::getIndexOf(const CString& value) const
{
	U pos = 0;

	for(auto it = Base::getBegin(); it != Base::getEnd(); ++it)
	{
		if(*it == value)
		{
			break;
		}
		++pos;
	}

	return (pos == Base::getSize()) ? -1 : pos;
}

} // end namespace anki
