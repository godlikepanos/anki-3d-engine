// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_ANIMATION_H
#define ANKI_RESOURCE_ANIMATION_H

#include "anki/Math.h"
#include "anki/resource/Common.h"
#include "anki/util/String.h"

namespace anki {

class XmlElement;

/// @addtogroup resource
/// @{

/// A keyframe 
template<typename T> 
class Key
{
	friend class Animation;

public:
	F32 getTime() const
	{
		return m_time;
	}

	const T& getValue() const
	{
		return m_value;
	}

private:
	F32 m_time;
	T m_value;
};

/// Animation channel
class AnimationChannel
{
public:
	BasicString<char, ResourceAllocator<char>> m_name;

	I32 m_boneIndex = -1; ///< For skeletal animations

	ResourceVector<Key<Vec3>> m_positions;
	ResourceVector<Key<Quat>> m_rotations;
	ResourceVector<Key<F32>> m_scales;
	ResourceVector<Key<F32>> m_cameraFovs;

	AnimationChannel(ResourceAllocator<U8>& alloc)
	:	m_name(alloc),
		m_positions(alloc),
		m_rotations(alloc),
		m_scales(alloc),
		m_cameraFovs(alloc)
	{}
};

/// Animation consists of keyframe data
class Animation
{
public:
	void load(const char* filename, ResourceAllocator<U8>& alloc);

	/// Get a vector of all animation channels
	const ResourceVector<AnimationChannel>& getChannels() const
	{
		return m_channels;
	}

	/// Get the duration of the animation in seconds
	F32 getDuration() const
	{
		return m_duration;
	}

	/// Get the time (in seconds) the animation should start
	F32 getStartingTime() const
	{
		return m_startTime;
	}

	/// The animation repeats
	Bool getRepeat() const
	{
		return m_repeat;
	}

	/// Get the interpolated data
	void interpolate(U channelIndex, F32 time, 
		Vec3& position, Quat& rotation, F32& scale) const;

private:
	ResourceVector<AnimationChannel> m_channels;
	F32 m_duration;
	F32 m_startTime;
	Bool8 m_repeat;

	void loadInternal(const XmlElement& el, ResourceAllocator<U8>& alloc);
};
/// @}

} // end namespace anki

#endif
