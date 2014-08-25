// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_ENUM_H
#define ANKI_UTIL_ENUM_H

#include "anki/util/StdTypes.h"

#define ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enum_, qualifier_) \
	qualifier_ enum_ operator|(enum_ a, enum_ b) \
	{ \
		return static_cast<enum_>(static_cast<U64>(a) | static_cast<U64>(b)); \
	} \
	qualifier_ void operator|=(enum_& a, enum_ b) \
	{ \
		a = static_cast<enum_>(static_cast<U64>(a) | static_cast<U64>(b)); \
	} \
	qualifier_ enum_ operator&(enum_ a, enum_ b) \
	{ \
		return static_cast<enum_>(static_cast<U64>(a) & static_cast<U64>(b)); \
	}

#endif


