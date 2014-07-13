// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_STRING_H
#define ANKI_STRING_H

#include "anki/util/Allocator.h"
#include <string>

namespace anki {

/// @addtogroup util_containers
/// @{

template<typename TChar, typename TAlloc>
using BasicString = std::basic_string<TChar, std::char_traits<TChar>, TAlloc>;

using String = BasicString<char, std::allocator<char>>;

/// Trim a string
/// Remove the @p what from the front and back of @p str
template<typename TString>
inline void trimString(const TString& in, const char* what, TString& out)
{
	out = in;
	out.erase(0, out.find_first_not_of(what));
	out.erase(out.find_last_not_of(what) + 1);
}

/// Replace substring. Substitute occurances of @a from into @a to inside the
/// @a str string
template<typename TString>
inline void replaceAllString(const TString& in, 
	const TString& from, const TString& to, TString& out)
{
	if(from.empty())
	{
		out = in;
	}

	out = in;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != TString::npos) 
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

/// @}

} // end namespace anki

#endif
