// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Math.h>
#include <anki/util/String.h>

namespace anki
{

// Forward
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
	String m_name;

	I32 m_boneIndex = -1; ///< For skeletal animations

	DynamicArray<Key<Vec3>> m_positions;
	DynamicArray<Key<Quat>> m_rotations;
	DynamicArray<Key<F32>> m_scales;
	DynamicArray<Key<F32>> m_cameraFovs;

	void destroy(ResourceAllocator<U8> alloc)
	{
		m_name.destroy(alloc);
		m_positions.destroy(alloc);
		m_rotations.destroy(alloc);
		m_scales.destroy(alloc);
		m_cameraFovs.destroy(alloc);
	}
};

/// Animation consists of keyframe data.
class Animation : public ResourceObject
{
public:
	Animation(ResourceManager* manager);

	~Animation();

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	/// Get a vector of all animation channels
	const DynamicArray<AnimationChannel>& getChannels() const
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
	void interpolate(U channelIndex, F32 time, Vec3& position, Quat& rotation, F32& scale) const;

private:
	DynamicArray<AnimationChannel> m_channels;
	F32 m_duration;
	F32 m_startTime;
	Bool8 m_repeat;
};
/// @}

} // end namespace anki
