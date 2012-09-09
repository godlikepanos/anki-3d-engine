#ifndef ANKI_CORE_TIMESTAMP_H
#define ANKI_CORE_TIMESTAMP_H

#include "anki/util/StdTypes.h"

namespace anki {

/// Give the current timestamp. It actually gives the current frame. Used to
/// indicate updates. It is actually returning the current frame
class Timestamp
{
public:
	static void increaseTimestamp()
	{
		++timestamp;
	}

	static U32 getTimestamp()
	{
		return timestamp;
	}

private:
	static U32 timestamp;
};

} // end namespace anki

#endif
