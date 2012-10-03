#include "anki/util/StringList.h"
#include <sstream>

namespace anki {

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
