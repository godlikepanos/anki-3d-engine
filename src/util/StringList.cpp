// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/StringList.h>

namespace anki
{

//==============================================================================
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

//==============================================================================
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
