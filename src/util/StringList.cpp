#include "anki/util/StringList.h"
#include <sstream>
#include <algorithm>

namespace anki {

//==============================================================================

static bool compareStringsAsc(const StringList::String& a, 
	const StringList::String& b)
{
	return a < b;
}

static bool compareStringsDesc(const StringList::String& a, 
	const StringList::String& b)
{
	return a > b;
}

//==============================================================================
StringList::String StringList::join(const Char* sep) const
{
	String out;

	Base::const_iterator it = begin();
	for(; it != end(); it++)
	{
		out += *it;
		if(it != end() - 1)
		{
			out += sep;
		}
	}

	return out;
}

//==============================================================================
I StringList::getIndexOf(const Char* value) const
{
	U32 pos = 0;

	for(const_iterator it = begin(); it != end(); ++it)
	{
		if(*it == value)
		{
			break;
		}
		++ pos;
	}

	return (pos == size()) ? -1 : pos;
}

//==============================================================================
void StringList::sortAll(const StringListSort method)
{
	if(method == SLS_ASCENDING)
	{
		std::sort(begin(), end(), compareStringsAsc);
	}
	else
	{
		std::sort(begin(), end(), compareStringsDesc);
	}
}

//==============================================================================
StringList StringList::splitString(const Char* s, Char seperator, Bool keep)
{
	StringList out;
	std::istringstream ss(s);
	while(!ss.eof())
	{
		String field;
		getline(ss, field, seperator);
		if(!keep && field.empty())
		{
			continue;
		}
		out.push_back(field);
	}

	return out;
}

//==============================================================================
std::ostream& operator<<(std::ostream& s, const StringList& a)
{
	s << a.join(", ");
	return s;
}

} // end namespace anki
