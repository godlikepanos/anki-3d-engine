// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>

namespace anki {

enum class AnimationBlendMode : U8
{
	kOverride, // Interpolate from the pose of the lower tracks to the pose of this track
	kAdditive, // Add the track's delta from the rest pose on top of the pose of the lower tracks

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AnimationBlendMode)
inline constexpr Array<const Char*, U32(AnimationBlendMode::kCount)> kAnimationBlendModeNames = {"Override", "Additive"};

enum class AnimationStart
{
	kNow, // Discard the queued animations of the track and blend from the current one
	kQueued, // Play after the queued animations of the track

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AnimationStart)

enum class AnimationPlayState : U8
{
	kStopped, // The track is like it doesn't exist
	kPaused, // The track's animation is frozen but it's used
	kPlaying, // Self explanatory

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AnimationPlayState)
inline constexpr Array<const Char*, U32(AnimationPlayState::kCount)> kAnimationPlayStateNames = {"Stopped", "Paused", "Playing"};

class AnimationState
{
public:
	// The time that has passed since the animation started. Try not to write to it. It starts at 0 and ends at
	// m_repeatTimes*animationResourceDuration
	Second m_animationTime = 0.0;

	// Time added to the time of the animation interpolation
	Second m_animationTimeBias = 0.0;

	// For example a value of 2.0 will play the animation in double speed
	F32 m_animationSpeedScale = 1.0f;

	// Negative means infinite
	F32 m_repeatTimes = 1.0f;
};

class AnimationTrackState
{
public:
	AnimationBlendMode m_blendMode = AnimationBlendMode::kOverride;

	// Time for the track to fade in over the lower tracks. Track 0 fades in from the rest pose
	Second m_blendInTime = 0.3;

	// Time for the track to fade out before its animation ends
	Second m_blendOutTime = 0.3;
};

template<typename T>
concept IsAnimationId = (std::is_integral_v<T> || std::is_enum_v<T>)&&sizeof(T) <= sizeof(U32);

} // end namespace anki
