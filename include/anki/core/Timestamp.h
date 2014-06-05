// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_TIMESTAMP_H
#define ANKI_CORE_TIMESTAMP_H

#include "anki/util/StdTypes.h"

namespace anki {

/// Timestamp type
typedef U32 Timestamp;

/// Increase the current timestamp. Should be called in the main loop
inline void increaseGlobTimestamp()
{
	extern Timestamp globTimestamp;
	++globTimestamp;
}

/// Give the current timestamp. It actually gives the current frame. Used to
/// indicate updates. It is actually returning the current frame
inline Timestamp getGlobTimestamp()
{
	extern Timestamp globTimestamp;
	return globTimestamp;
}

} // end namespace anki

#endif
