#ifndef ANKI_UTIL_STRING_LIST_H
#define ANKI_UTIL_STRING_LIST_H

#include <vector>
#include <string>


namespace anki {


/// A simple convenience class
class StringList: public std::vector<std::string>
{
public:
	typedef std::vector<std::string> Base; ///< Its the vector of strings
	typedef Base::value_type StringType; ///< Its string

	/// Return the list as a single string
	StringType join(const StringType& sep) const;

	/// XXX
	static StringList splitString(const StringType& s,
		const char* sep = " ");
};


} // end namespace


#endif
