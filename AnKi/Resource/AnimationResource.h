// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Forward
class XmlElement;

/// @addtogroup resource
/// @{

/// A keyframe
template<typename T>
class AnimationKeyframe
{
	friend class AnimationResource;

public:
	Second getTime() const
	{
		return m_time;
	}

	const T& getValue() const
	{
		return m_value;
	}

private:
	Second m_time;
	T m_value;
};

/// Animation channel
class AnimationChannel
{
public:
	ResourceString m_name;

	I32 m_boneIndex = -1; ///< For skeletal animations

	ResourceDynamicArray<AnimationKeyframe<Vec3>> m_positions;
	ResourceDynamicArray<AnimationKeyframe<Quat>> m_rotations;
	ResourceDynamicArray<AnimationKeyframe<F32>> m_scales;
	ResourceDynamicArray<AnimationKeyframe<F32>> m_cameraFovs;
};

/// Animation consists of keyframe data.
class AnimationResource : public ResourceObject
{
public:
	AnimationResource() = default;

	~AnimationResource() = default;

	Error load(const ResourceFilename& filename, Bool async);

	/// Get a vector of all animation channels
	ConstWeakArray<AnimationChannel> getChannels() const
	{
		return m_channels;
	}

	/// Get the duration of the animation in seconds
	Second getDuration() const
	{
		return m_duration;
	}

	/// Get the time (in seconds) the animation should start
	Second getStartingTime() const
	{
		return m_startTime;
	}

	/// Get the interpolated data
	void interpolate(U32 channelIndex, Second time, Vec3& position, Quat& rotation, F32& scale) const;

private:
	ResourceDynamicArray<AnimationChannel> m_channels;
	Second m_duration;
	Second m_startTime;
};
/// @}

} // end namespace anki
