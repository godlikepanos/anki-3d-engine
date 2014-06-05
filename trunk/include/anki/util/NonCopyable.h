// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_NON_COPYABLE_H
#define ANKI_UTIL_NON_COPYABLE_H

namespace anki {

/// @addtogroup util_patterns
/// @{

/// Makes a derived class non copyable
struct NonCopyable
{
	NonCopyable()
	{}

	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;

	~NonCopyable()
	{}
};
/// @}

} // end namespace anki

#endif
