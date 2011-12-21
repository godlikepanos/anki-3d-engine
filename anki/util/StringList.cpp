#include "anki/util/StringList.h"
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
StringList::StringType StringList::join(const StringType& sep) const
{
	StringType out;

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
StringList StringList::splitString(const StringType& s, const char* seperators)
{
	typedef boost::char_separator<char> Sep;
	typedef boost::tokenizer<Sep> Tok;

	Sep sep(seperators);
	StringList out;
	Tok tok(s, sep);

	BOOST_FOREACH(const std::string& s, tok)
	{
		out.push_back(s);
	}

	return out;
}


} // end namespace
