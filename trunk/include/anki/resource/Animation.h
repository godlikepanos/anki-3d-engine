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
	const Vector<AnimationChannel>& getChannels() const
	{
		return channels;
	}

	F32 getDuration() const
	{
		return duration;
	}

	F32 getStartingTime() const
	{
		return startTime;
	}
	/// @}

	/// Get the interpolated data
	void interpolate(U channelIndex, F32 time, 
		Vec3& position, Quat& rotation, F32& scale) const;

private:
	Vector<AnimationChannel> channels;
	F32 duration;
	F32 startTime;

	void loadInternal(const XmlElement& el);
};
/// @}

} // end namespace anki

#endif
