#ifndef ANKI_RESOURCE_ANIMATION_H
#define ANKI_RESOURCE_ANIMATION_H

#include "anki/Math.h"
#include "anki/util/Vector.h"

namespace anki {

class XmlElement;

/// @addtogroup Resources
/// @{

/// A keyframe 
template<typename T> 
class Key
{
	friend class Animation;

public:
	F32 getTime() const
	{
		return time;
	}

	const T& getValue() const
	{
		return value;
	}

private:
	F32 time;
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

/// Animation consists of keyframe data
class Animation
{
public:
	void load(const char* filename);

	/// @name Accessors
	/// @{

	/// Get a vector of all animation channels
	const Vector<AnimationChannel>& getChannels() const
	{
		return channels;
	}

	/// Get the duration of the animation in seconds
	F32 getDuration() const
	{
		return duration;
	}

	/// Get the time (in seconds) the animation should start
	F32 getStartingTime() const
	{
		return startTime;
	}

	/// The animation repeats
	Bool getRepeat() const
	{
		return repeat;
	}
	/// @}

	/// Get the interpolated data
	void interpolate(U channelIndex, F32 time, 
		Vec3& position, Quat& rotation, F32& scale) const;

private:
	Vector<AnimationChannel> channels;
	F32 duration;
	F32 startTime;
	Bool8 repeat;

	void loadInternal(const XmlElement& el);
};
/// @}

} // end namespace anki

#endif
