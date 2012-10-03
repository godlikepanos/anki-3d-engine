#ifndef ANKI_UTIL_STRING_LIST_H
#define ANKI_UTIL_STRING_LIST_H

#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include <string>

namespace anki {

/// @addtogroup util
/// @{

/// A simple convenience class for string lists
class StringList: public Vector<std::string>
{
public:
	typedef StringList Self; ///< Self type
	typedef Vector<std::string> Base; ///< Its the vector of strings
	typedef Base::value_type String; ///< Its string
	typedef String::value_type Char; ///< Char type

	/// Join all the elements into a single big string using a the
	/// seperator @a sep
	String join(const Char* sep) const;

	/// Returns the index position of the last occurrence of @a value in
	/// the list
	/// @return -1 of not found
	I getIndexOf(const Char* value) const;

	/// Split a string using a list of separators (@a sep) and return these
	/// strings in a string list
	static Self splitString(const Char* s, const Char sep = ' ',
		Bool keepEmpties = false);

	/// Mainly for the unit tests
	friend std::ostream& operator<<(std::ostream& s, const Self& a);
};
/// @}

} // end namespace anki

#endif
