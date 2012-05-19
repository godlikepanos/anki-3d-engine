#include "anki/util/StringList.h"
#include <iostream> ///XXX
#include <boost/tokenizer.hpp> /// XXX Remove

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
int StringList::getIndexOf(const Char* value) const
{
	size_t pos = 0;

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
StringList StringList::splitString(const Char* s, const Char* seperators)
{
	typedef boost::char_separator<Char> Sep;
	typedef boost::tokenizer<Sep> Tok;

	Sep sep(seperators);
	StringList out;
	String str(s);
	Tok tok(str, sep);

	for(const String& s_ : tok)
	{
		out.push_back(s_);
	}

	return out;
}

//==============================================================================
std::ostream& operator<<(std::ostream& s, const StringList& a)
{
	s << a.join(", ");
	return s;
}

} // end namespace
