// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/StringList.h>

namespace anki {

template<typename TMemoryPool>
template<typename TStringMemoryPool>
void BaseStringList<TMemoryPool>::join(const CString& separator, BaseString<TStringMemoryPool>& out) const
{
	if(Base::isEmpty())
	{
		return;
	}

	// Count the characters
	const I sepLen = separator.getLength();
	I charCount = 0;
	for(const auto& str : *this)
	{
		charCount += str.getLength() + sepLen;
	}

	charCount -= sepLen; // Remove last separator
	ANKI_ASSERT(charCount > 0);

	// Allocate
	out = std::move(BaseString<TStringMemoryPool>('?', charCount, out.getMemoryPool()));

	// Append to output
	Char* to = &out[0];
	typename Base::ConstIterator it = Base::getBegin();
	for(; it != Base::getEnd(); it++)
	{
		const auto& from = *it;
		memcpy(to, &from[0], from.getLength() * sizeof(Char));
		to += from.getLength();

		if(it != Base::end() - 1)
		{
			memcpy(to, &separator[0], sepLen * sizeof(Char));
			to += sepLen;
		}
	}
}

template<typename TMemoryPool>
I BaseStringList<TMemoryPool>::getIndexOf(const CString& value) const
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

template<typename TMemoryPool>
void BaseStringList<TMemoryPool>::sortAll(const Sort method)
{
	if(method == Sort::kAscending)
	{
		Base::sort([](const StringType& a, const StringType& b) {
			return a < b;
		});
	}
	else
	{
		ANKI_ASSERT(method == Sort::kDescending);
		Base::sort([](const StringType& a, const StringType& b) {
			return a > b;
		});
	}
}

template<typename TMemoryPool>
void BaseStringList<TMemoryPool>::splitString(const CString& s, const Char separator, Bool keepEmpty)
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
				Base::emplaceBack(std::move(StringType(begin, end, Base::getMemoryPool())));
			}

			break;
		}
		else if(*end == separator)
		{
			if(begin < end)
			{
				Base::emplaceBack(StringType(begin, end, Base::getMemoryPool()));

				begin = end + 1;
			}
			else
			{
				if(keepEmpty)
				{
					Base::emplaceBack(Base::getMemoryPool());
				}

				++begin;
			}
		}

		++end;
	}
}

template<typename TMemoryPool>
void BaseStringList<TMemoryPool>::pushBackSprintf(const Char* fmt, ...)
{
	StringType str(Base::getMemoryPool());
	va_list args;
	va_start(args, fmt);
	str.sprintfInternal(fmt, args);
	va_end(args);

	Base::emplaceBack(std::move(str));
}

template<typename TMemoryPool>
void BaseStringList<TMemoryPool>::pushFrontSprintf(const Char* fmt, ...)
{
	StringType str(Base::getMemoryPool());
	va_list args;
	va_start(args, fmt);
	str.sprintfInternal(fmt, args);
	va_end(args);

	Base::emplaceFront(std::move(str));
}

} // end namespace anki
