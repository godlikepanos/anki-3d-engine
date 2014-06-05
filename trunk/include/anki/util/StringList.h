// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_STRING_LIST_H
#define ANKI_UTIL_STRING_LIST_H

#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/String.h"
#include <sstream>
#include <algorithm>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Sort method for StringList
enum class StringListSort
{
	ASCENDING,
	DESCENDING
};

/// A simple convenience class for string lists
template<typename TChar, typename TAlloc>
class BasicStringList: public Vector<BasicString<TChar, TAlloc>, TAlloc>
{
public:
	typedef BasicStringList Self; ///< Self type
	/// Its the vector of strings
	typedef Vector<BasicString<TChar, TAlloc>, TAlloc> Base; 
	typedef typename Base::value_type String; ///< Its string
	typedef typename String::value_type Char; ///< Char type

	/// Join all the elements into a single big string using a the
	/// seperator @a separator
	String join(const Char* separator) const
	{
		String out;

		typename Base::const_iterator it = Base::begin();
		for(; it != Base::end(); it++)
		{
			out += *it;
			if(it != Base::end() - 1)
			{
				out += separator;
			}
		}

		return out;
	}

	/// Returns the index position of the last occurrence of @a value in
	/// the list
	/// @return -1 of not found
	I getIndexOf(const Char* value) const
	{
		U32 pos = 0;

		for(auto it = Base::begin(); it != Base::end(); ++it)
		{
			if(*it == value)
			{
				break;
			}
			++ pos;
		}

		return (pos == Base::size()) ? -1 : pos;
	}

	/// Sort the string list
	void sortAll(const StringListSort method = StringListSort::ASCENDING)
	{
		if(method == StringListSort::ASCENDING)
		{
			std::sort(Base::begin(), Base::end(), compareStringsAsc);
		}
		else
		{
			ANKI_ASSERT(method == StringListSort::DESCENDING);
			std::sort(Base::begin(), Base::end(), compareStringsDesc);
		}
	}

	/// Split a string using a list of separators (@a sep) and return these
	/// strings in a string list
	static Self splitString(const Char* s, const Char separator = ' ',
		Bool keepEmpties = false)
	{
		Self out;
		std::istringstream ss(s);
		while(!ss.eof())
		{
			String field;
			getline(ss, field, separator);
			if(!keepEmpties && field.empty())
			{
				continue;
			}
			out.push_back(field);
		}

		return out;
	}

	/// Mainly in the unit tests
	friend std::ostream& operator<<(std::ostream& s, const Self& a)
	{
		s << a.join(", ");
		return s;
	}

private:
	static Bool compareStringsAsc(const String& a, const String& b)
	{
		return a < b;
	}

	static Bool compareStringsDesc(const String& a,	const String& b)
	{
		return a > b;
	}
};

/// A common string list allocated in heap
typedef BasicStringList<char, std::allocator<char>> StringList;
/// @}

} // end namespace anki

#endif
