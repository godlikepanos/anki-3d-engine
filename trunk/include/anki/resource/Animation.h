#ifndef ANKI_RESOURCE_ANIMATION_H
#define ANKI_RESOURCE_ANIMATION_H

#include "anki/Math.h"
#include "anki/util/Vector.h"

namespace anki {

/// A keyframe 
template<typename T> 
class Key
{
	friend class Animation;

public:
	F64 getTime() const
	{
		return time;
	}

	const T& getValue() const
	{
		return value;
	}

private:
	F64 time;
	T value;
};

/// Animation channel
struct AnimationChannel
{
	std::string name;

	I32 boneIndex = -1; ///< For skeletal animations

	Vector<Key<Vec3>> positions;
	Vector<Key<Quat>> rotations;
	Vector<Key<F32>> scales;
	Vector<Key<F32>> cameraFovs;
};

/// Animation
class Animation
{
public:
	void load(const char* filename);

	const Vector<AnimationChannel>& getChannels() const
	{
		return channels;
	}

private:
	Vector<AnimationChannel> channels;
};

} // end namespace anki

#endif
