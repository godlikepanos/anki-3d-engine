// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/StringList.h>

namespace anki {

template<typename TMemPool>
void StringList::destroy(TMemPool& pool)
{
	auto it = Base::getBegin();
	auto endit = Base::getEnd();
	for(; it != endit; ++it)
	{
		(*it).destroy(pool);
	}

	Base::destroy(pool);
}

template<typename TMemPool>
void StringList::join(const CString& separator, BaseStringRaii<TMemPool>& out) const
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

template<typename TMemPool>
void StringList::splitString(TMemPool& pool, const CString& s, const Char separator, Bool keepEmpty)
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
				Base::emplaceBack(pool);

				String str;
				str.create(pool, begin, end);
				Base::getBack() = std::move(str);
			}

			break;
		}
		else if(*end == separator)
		{
			if(begin < end)
			{
				Base::emplaceBack(pool);

				String str;
				str.create(pool, begin, end);

				Base::getBack() = std::move(str);
				begin = end + 1;
			}
			else
			{
				if(keepEmpty)
				{
					Base::emplaceBack(pool);
				}

				++begin;
			}
		}

		++end;
	}
}

template<typename TMemPool>
void StringList::pushBackSprintf(TMemPool& pool, const Char* fmt, ...)
{
	String str;
	va_list args;
	va_start(args, fmt);
	str.sprintfInternal(pool, fmt, args);
	va_end(args);

	Base::emplaceBack(pool);
	Base::getBack() = std::move(str);
}

template<typename TMemPool>
void StringList::pushFrontSprintf(TMemPool& pool, const Char* fmt, ...)
{
	String str;
	va_list args;
	va_start(args, fmt);
	str.sprintfInternal(pool, fmt, args);
	va_end(args);

	Base::emplaceFront(pool);
	Base::getFront() = std::move(str);
}

template<typename TMemPool>
void BaseStringListRaii<TMemPool>::pushBackSprintf(const Char* fmt, ...)
{
	String str;
	va_list args;
	va_start(args, fmt);
	str.sprintfInternal(m_pool, fmt, args);
	va_end(args);

	Base::emplaceBack(m_pool);
	Base::getBack() = std::move(str);
}

template<typename TMemPool>
void BaseStringListRaii<TMemPool>::pushFrontSprintf(const Char* fmt, ...)
{
	String str;
	va_list args;
	va_start(args, fmt);
	str.sprintfInternal(m_pool, fmt, args);
	va_end(args);

	Base::emplaceFront(m_pool);
	Base::getFront() = std::move(str);
}

} // end namespace anki
