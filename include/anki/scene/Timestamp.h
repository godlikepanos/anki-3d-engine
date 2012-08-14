#ifndef ANKI_SCENE_TIMESTAMP_H
#define ANKI_SCENE_TIMESTAMP_H

#include <cstdint>

namespace anki {

/// Give the current timestamp. It actualy gives the current frame. Normally it
/// should have been part of the Scene class but its a different class because 
/// we don't want to include the whole Scene.h in those classes that just need 
/// the timestamp
class Timestamp
{
public:
	static void increaseTimestamp()
	{
		++timestamp;
	}

	static uint32_t getTimestamp()
	{
		return timestamp;
	}

private:
	static uint32_t timestamp;
};

} // end namespace anki

#endif
