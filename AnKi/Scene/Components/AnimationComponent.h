// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>

namespace anki {

enum class AnimationWrapMode : U8
{
	kOnce,
	kLoop,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AnimationWrapMode)
inline constexpr Array<const Char*, U32(AnimationWrapMode::kCount)> kAnimationWrapModeNames = {"Once", "Loop"};

enum class AnimationBlendMode : U8
{
	kBlend,
	kAdditive,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AnimationBlendMode)
inline constexpr Array<const Char*, U32(AnimationBlendMode::kCount)> kAnimationBlendModeNames = {"Blend", "Additive"};

enum class AnimationState : U8
{
	kStopped, // The track is like it doesn't exist
	kPaused, // The track's animation is frozen but it's used
	kPlaying, // Self explanatory

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AnimationState)
inline constexpr Array<const Char*, U32(AnimationState::kCount)> kAnimationStateNames = {"Stopped", "Paused", "Playing"};

// Animation component. Plays animations that affect the attached node.
class AnimationComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(AnimationComponent)

public:
	static constexpr U32 kMaxAnimationTracks = 4;

	AnimationComponent(const SceneComponentInitInfo& init);

	~AnimationComponent();

	AnimationComponent& setAnimationFilename(U32 track, CString fname);

	CString getAnimationFilename(U32 track) const;

	Bool hasAnimationFilename(U32 track) const
	{
		return ANKI_EXPECT(track < kMaxAnimationTracks) ? !!m_tracks[track].m_anim : false;
	}

	// Need to call this after you've set an animation filename.
	AnimationComponent& setAnimationChannel(U32 track, CString channel);

	CString getAnimationChannel(U32 track) const;

	AnimationComponent& setAnimationBlendWeight(U32 track, F32 blend)
	{
		if(ANKI_EXPECT(track < kMaxAnimationTracks))
		{
			m_tracks[track].m_blendWeight = clamp(blend, 0.01f, 1.0f);
		}
		return *this;
	}

	F32 getAnimationBlendWeight(U32 track) const
	{
		return (ANKI_EXPECT(track < kMaxAnimationTracks)) ? m_tracks[track].m_blendWeight : 0.0f;
	}

	AnimationComponent& setAnimationWrapMode(U32 track, AnimationWrapMode mode)
	{
		if(ANKI_EXPECT(track < kMaxAnimationTracks && mode < AnimationWrapMode::kCount))
		{
			m_tracks[track].m_wrapMode = mode;
		}
		return *this;
	}

	AnimationWrapMode getAnimationWrapMode(U32 track) const
	{
		return (ANKI_EXPECT(track < kMaxAnimationTracks)) ? m_tracks[track].m_wrapMode : AnimationWrapMode::kFirst;
	}

	AnimationComponent& setAnimationBlendMode(U32 track, AnimationBlendMode mode)
	{
		if(ANKI_EXPECT(track < kMaxAnimationTracks && mode < AnimationBlendMode::kCount))
		{
			m_tracks[track].m_blendMode = mode;
		}
		return *this;
	}

	AnimationBlendMode getAnimationBlendMode(U32 track) const
	{
		return (ANKI_EXPECT(track < kMaxAnimationTracks)) ? m_tracks[track].m_blendMode : AnimationBlendMode::kFirst;
	}

	AnimationComponent& setAnimationSpeed(U32 track, F32 speed)
	{
		// Speed can be negative to play the animation in reverse, so no clamping
		if(ANKI_EXPECT(track < kMaxAnimationTracks))
		{
			m_tracks[track].m_animationSpeedScale = speed;
		}
		return *this;
	}

	F32 getAnimationSpeed(U32 track) const
	{
		return (ANKI_EXPECT(track < kMaxAnimationTracks)) ? m_tracks[track].m_animationSpeedScale : 1.0f;
	}

	void resetTrack(U32 track);

	AnimationComponent& setAnimationState(U32 track, AnimationState state)
	{
		if(ANKI_EXPECT(track < kMaxAnimationTracks) && m_tracks[track].m_state != state)
		{
			m_tracks[track].m_state = state;
			if(state == AnimationState::kStopped)
			{
				m_tracks[track].m_relativeTimePassed = 0.0;
			}
		}
		return *this;
	}

	AnimationState getAnimationState(U32 track) const
	{
		return (ANKI_EXPECT(track < kMaxAnimationTracks)) ? m_tracks[track].m_state : AnimationState::kFirst;
	}

#if ANKI_WITH_EDITOR
	AnimationComponent& setEditorAnimationState(U32 track, AnimationState state)
	{
		if(ANKI_EXPECT(track < kMaxAnimationTracks) && m_tracks[track].m_editorState != state)
		{
			m_tracks[track].m_editorState = state;
			if(state == AnimationState::kStopped)
			{
				m_tracks[track].m_relativeTimePassed = 0.0;
			}
		}
		return *this;
	}

	AnimationState getEditorAnimationState(U32 track) const
	{
		return (ANKI_EXPECT(track < kMaxAnimationTracks)) ? m_tracks[track].m_editorState : AnimationState::kFirst;
	}
#endif

	Bool isValid() const
	{
		return m_validTracks.getAnySet();
	}

private:
	class Track
	{
	public:
		Second m_relativeTimePassed = 0.0;
		AnimationResourcePtr m_anim;
		U32 m_channel = 0;
		F32 m_animationSpeedScale = 1.0f;
		F32 m_blendWeight = 1.0f;
		AnimationState m_state = AnimationState::kStopped;
#if ANKI_WITH_EDITOR
		AnimationState m_editorState = AnimationState::kStopped;
#endif
		AnimationWrapMode m_wrapMode = AnimationWrapMode::kLoop;
		AnimationBlendMode m_blendMode = AnimationBlendMode::kBlend;
	};

	Array<Track, kMaxAnimationTracks> m_tracks;
	BitSet<kMaxAnimationTracks, U8> m_validTracks = {false};

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
