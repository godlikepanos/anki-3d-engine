// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/AnimationResource.h>

namespace anki {

enum class NodeMotionMode : U8
{
	// The animation plays its transform alters the local transform of the node. In that mode the animation moves the node in the world
	kApplyToNode,

	// The animation plays and the motion is returned in deltas, node's transform is not touched. The node doesn't move but the motion is returned
	// instead
	kExtract,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(NodeMotionMode)

// Animation component. Plays animations that affect the attached node.
class AnimationComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(AnimationComponent)

public:
	static constexpr U32 kMaxAnimationTracks = 4;
	static constexpr U32 kMaxAnimationsPerTrack = 4;

	AnimationComponent(const SceneComponentInitInfo& init);

	~AnimationComponent();

	// Loads an animation resource and associates it with the animId. Can be something like registerAnimation(kWalkAnimation, "Walk.ankianim")
	template<typename T>
	void registerAnimation(T animId, CString animationFilepath) requires(IsAnimationId<T>)
	{
		registerAnimationInternal(U32(animId), animationFilepath);
	}

	// Remove the ref of the loaded animation
	template<typename T>
	void unregisterAnimation(T animId) requires(IsAnimationId<T>)
	{
		unregisterAnimationInternal(U32(animId));
	}

	// animId: The same id passed in registerAnimation()
	// channel: The name of the channel or empty for the 1st channel
	// blendInTime: The time from when the animation starts until it fully replaces the previous animation in the track
	// Returns the state of the new animation. Don't keep it around: when the animation finishes its slot gets reused by other animations
	template<typename T>
	AnimationState& playAnimation(U32 track, T animId, CString channel, Second blendInTime, AnimationStart animStart) requires(IsAnimationId<T>)
	{
		return playAnimationInternal(track, U32(animId), channel, blendInTime, animStart);
	}

	// Get the animation state of the first queued animation with the given ID
	template<typename T>
	AnimationState& getAnimationState(U32 track, T animId) requires(IsAnimationId<T>)
	{
		return getAnimationStateInternal(track, U32(animId));
	}

	template<typename T>
	const AnimationState& getAnimationState(U32 track, T animId) const requires(IsAnimationId<T>)
	{
		return const_cast<AnimationComponent*>(this)->getAnimationState(track, animId);
	}

	AnimationTrackState& getAnimationTrackState(U32 track)
	{
		if(!ANKI_EXPECT(track < kMaxAnimationTracks))
		{
			track = kMaxAnimationTracks - 1;
		}
		return m_tracks[track].m_state;
	}

	const AnimationTrackState& getAnimationTrackState(U32 track) const
	{
		return const_cast<AnimationComponent*>(this)->getAnimationTrackState(track);
	}

	Bool getTrackHasAnimations(U32 track) const
	{
		if(ANKI_EXPECT(track < kMaxAnimationTracks))
		{
			return m_tracks[track].m_crntAnimationIdx != m_tracks[track].m_lastAnimationIdx
				   || !!m_tracks[track].m_anims[m_tracks[track].m_crntAnimationIdx % kMaxAnimationsPerTrack].m_resource;
		}
		else
		{
			return false;
		}
	}

	void resetTrack(U32 track);

	// Returns the transform delta of the animation since the last update
	Transform getNodeTransformDelta() const
	{
		return (m_motionMode == NodeMotionMode::kExtract) ? m_nodeTransformDelta : Transform::getIdentity();
	}

	void setNodeMotionMode(NodeMotionMode mode)
	{
		if(ANKI_EXPECT(mode < NodeMotionMode::kCount))
		{
			m_motionMode = mode;
		}
	}

	NodeMotionMode getNodeMotionMode() const
	{
		return m_motionMode;
	}

	Bool isValid() const
	{
		return true;
	}

	ANKI_INTERNAL ConstWeakArray<std::pair<U32, U32>> getAnimationsStarted() const
	{
		return m_animationsStarted;
	}

	ANKI_INTERNAL ConstWeakArray<std::pair<U32, U32>> getAnimationsEnded() const
	{
		return m_animationsEnded;
	}

	ANKI_INTERNAL CString getFilepathOfFirstAnimationPlaying(U32 track) const;

#if ANKI_WITH_EDITOR
	ANKI_INTERNAL Bool getPlayOnEditor() const
	{
		return m_playOnEditor;
	}

	ANKI_INTERNAL void setPlayOnEditor(Bool play)
	{
		m_playOnEditor = play;
	}
#endif

private:
	class Trf
	{
	public:
		Vec3 m_pos;
		Quat m_rot;
		F32 m_scale = 1.0f;
	};

	class Animation
	{
	public:
		AnimationResourcePtr m_resource;
		Second m_blendInTime = 0.0f;
		AnimationStart m_start = AnimationStart::kNow;
		U32 m_animId = kMaxU32;
		U32 m_channelIdx = kMaxU32;

		AnimationState m_state;

		Second m_interpolationAnimTime = kMaxSecond; // Tracking the time passed to AnimationResource::interpolate()

		Trf m_animTrf; // Tracking the transform of the animation

		Bool m_started = false;
	};

	class Track
	{
	public:
		Array<Animation, kMaxAnimationsPerTrack> m_anims;

		AnimationTrackState m_state;

		U32 m_crntAnimationIdx = 0; // The animation that plays now
		U32 m_lastAnimationIdx = 0; // The last queued animation
	};

	Array<Track, kMaxAnimationTracks> m_tracks;

	SceneHashMap<U32, std::pair<AnimationResourcePtr, U32>> m_idToAnimationResource;

	AnimationState m_dummyState;

	// The events of the last update. Read by the callbacks
	SceneDynamicArray<std::pair<U32, U32>> m_animationsStarted;
	SceneDynamicArray<std::pair<U32, U32>> m_animationsEnded;

	// The events gathered until the end of the update. Separate from the above so callbacks that call playAnimation() don't modify what they iterate
	SceneDynamicArray<std::pair<U32, U32>> m_pendingAnimationsStarted;
	SceneDynamicArray<std::pair<U32, U32>> m_pendingAnimationsEnded;

	// For node motion calculation:
	Transform m_nodeTransformDelta;
	NodeMotionMode m_motionMode = NodeMotionMode::kApplyToNode;

#if ANKI_WITH_EDITOR
	Bool m_playOnEditor = false;
#endif

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;

	void registerAnimationInternal(U32 animId, CString animationFilepath);

	void unregisterAnimationInternal(U32 animId);

	AnimationState& playAnimationInternal(U32 track, U32 animId, CString channel, Second blendInTime, AnimationStart animStart);

	AnimationState& getAnimationStateInternal(U32 trackIdx, U32 animId);
};

} // end namespace anki
