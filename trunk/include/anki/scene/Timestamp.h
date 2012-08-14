#ifndef ANKI_SCENE_TIMESTAMP_H
#define ANKI_SCENE_TIMESTAMP_H

namespace anki {

/// Give the current timestamp. It actualy gives the current frame. Normaly it 
/// should have been part of the Scene class but its a different class because 
/// we don't want to include the whole Scene.h in those classes that just need 
/// the timestamp
class Timestamp
{
public:
	static int increaseTimestamp()
	{
		++timestamp;
	}

	static int getTimestamp()
	{
		return timestamp;
	}

private:
	static int timestamp;
};

} // end namespace anki

#endif
