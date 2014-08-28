// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_STRING_H
#define ANKI_STRING_H

#include "anki/util/Allocator.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/util/Array.h"
#include <string>

namespace anki {

/// @addtogroup util_private
/// @{

namespace detail {

template<typename TNumber>
constexpr const char* toStringFormat()
{
	return nullptr;
}

#define ANKI_DEPLOY_TO_STRING(type_, string_) \
	template<> \
	constexpr const char* toStringFormat<type_>() \
	{ \
		return string_; \
	}

ANKI_DEPLOY_TO_STRING(I8, "%d")
ANKI_DEPLOY_TO_STRING(I16, "%d")
ANKI_DEPLOY_TO_STRING(I32, "%d")
ANKI_DEPLOY_TO_STRING(I64, "%lld")
ANKI_DEPLOY_TO_STRING(U8, "%u")
ANKI_DEPLOY_TO_STRING(U16, "%u")
ANKI_DEPLOY_TO_STRING(U32, "%u")
ANKI_DEPLOY_TO_STRING(U64, "%llu")
ANKI_DEPLOY_TO_STRING(F32, "%f")
ANKI_DEPLOY_TO_STRING(F64, "%f")

#undef ANKI_DEPLOY_TO_STRING

} // end namespace detail

/// @}

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

/// To string implementation
template<typename TNumber, typename TString>
inline void toString(TNumber number, TString& out)
{
	Array<typename TString::value_type, 512> buff;
	I ret = std::snprintf(
		&buff[0], buff.size(), detail::toStringFormat<TNumber>(), number);

	if(ret < 0 || ret > static_cast<I>(buff.size()))
	{
		throw ANKI_EXCEPTION("To small intermediate buffer");
	}
}

/// @}

} // end namespace anki

#endif
