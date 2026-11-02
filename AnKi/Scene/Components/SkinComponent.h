// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/Forward.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Math.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>

namespace anki {

enum class RootMotionMode : U8
{
	// Pose untouched and root motion delta will be 0
	kDisabled,

	// Horizontal root bone motion removed and returned as root motion delta. Vertical works as kDisabled. Used in locomotion where we want the root
	// motion delta to not consider Y axis
	kHorizontal,

	// Same as kHorizontal plus it computes the root rotation. Useful in locomotion that contains turns
	kHorizontalAndYaw,

	// Root motion stays locked in pose. Useful in animations that have root motion that we want to cancel. For example, a jump animation moves the
	// character up. But we want the physics to drive the motion upwards, not the animation. In this case we can disable root motion
	kLockRoot,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RootMotionMode)

template<typename T>
concept IsAnimationId = (std::is_integral_v<T> || std::is_enum_v<T>)&&sizeof(T) <= sizeof(U32);

// Skin component
class SkinComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SkinComponent)

public:
	static constexpr U32 kMaxAnimationTracks = 4;
	static constexpr U32 kMaxAnimationsPerTrack = 4;

	SkinComponent(const SceneComponentInitInfo& init);

	~SkinComponent();

	SkinComponent& setSkeletonFilename(CString filename);

	CString getSkeletonFilename() const;

	Bool hasSkeletonResource() const
	{
		return !!m_resource;
	}

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
	// blendInTime: The time from when the animation starts until it fully replaces the previous animation in the track
	// Returns the state of the new animation. Don't keep it around: when the animation finishes its slot gets reused by other animations
	template<typename T>
	AnimationState& playAnimation(U32 track, T animId, Second blendInTime, AnimationStart animStart) requires(IsAnimationId<T>)
	{
		return playAnimationInternal(track, U32(animId), blendInTime, animStart);
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
		return const_cast<SkinComponent*>(this)->getAnimationState(track, animId);
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
		return const_cast<SkinComponent*>(this)->getAnimationTrackState(track);
	}

	// Returns the translation delta of the root bone since the last update
	Vec3 getRootMotionDelta() const
	{
		return (m_rootMotionMode == RootMotionMode::kHorizontal || m_rootMotionMode == RootMotionMode::kHorizontalAndYaw) ? m_rootMotionDelta
																														  : Vec3(0.0f);
	}

	// Returns the angle of the root bone
	Mat3 getRootYawDelta() const
	{
		if(m_rootMotionMode == RootMotionMode::kHorizontalAndYaw)
		{
			return Mat3(Axisang(m_rootYawDeltaAngle, m_skeletonUp));
		}
		else
		{
			return Mat3::getIdentity();
		}
	}

	void setRootMotionMode(RootMotionMode mode)
	{
		if(ANKI_EXPECT(mode < RootMotionMode::kCount))
		{
			m_rootMotionMode = mode;
		}
	}

	RootMotionMode getRootMotionMode() const
	{
		return m_rootMotionMode;
	}

	Bool isValid() const
	{
		return m_resource.isCreated() && m_boneTrfs[0].getSize() > 0;
	}

	ANKI_INTERNAL ConstWeakArray<Mat3x4> getBoneTransforms() const
	{
		ANKI_ASSERT(isValid());
		return m_boneTrfs[m_crntBoneTrfs];
	}

	ANKI_INTERNAL ConstWeakArray<Mat3x4> getPreviousFrameBoneTransforms() const
	{
		ANKI_ASSERT(isValid());
		return m_boneTrfs[m_prevBoneTrfs];
	}

	ANKI_INTERNAL const SkeletonResourcePtr& getSkeletonResource() const
	{
		ANKI_ASSERT(isValid());
		return m_resource;
	}

	ANKI_INTERNAL const Aabb& getBoneBoundingVolumeLocalSpace() const
	{
		ANKI_ASSERT(isValid());
		return m_boneBoundingVolume;
	}

	ANKI_INTERNAL U32 getBoneTransformsGpuSceneOffset() const
	{
		ANKI_ASSERT(isValid());
		return U32(m_gpuSceneBoneTransforms.getOffset());
	}

	ANKI_INTERNAL Bool gpuSceneReallocationsThisFrame() const
	{
		ANKI_ASSERT(isValid());
		return m_boneTransformsReallocatedThisFrame;
	}

	ANKI_INTERNAL SceneNode& getSceneNode() const
	{
		return *m_node;
	}

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
	class Animation
	{
	public:
		AnimationResourcePtr m_resource;
		Second m_blendInTime = 0.0;
		AnimationStart m_start = AnimationStart::kNow;
		U32 m_animId = kMaxU32;

		AnimationState m_state;

		Second m_interpolationAnimTime = kMaxSecond; // Tracking the time passed to AnimationResource::interpolate()
		Vec3 m_rootPosition = Vec3(0.0f); // The position of the root bone
		Quat m_rootRotation = Quat::getIdentity();
	};

	class Track
	{
	public:
		Array<Animation, kMaxAnimationsPerTrack> m_anims;

		AnimationTrackState m_state;

		U32 m_crntAnimationIdx = 0; // The animation that plays now
		U32 m_lastAnimationIdx = 0; // The last queued animation
	};

	class Trf
	{
	public:
		Vec3 m_translation;
		Quat m_rotation;
		F32 m_scale;
	};

	SceneNode* m_node = nullptr;
	SkeletonResourcePtr m_resource;
	Array<SceneDynamicArray<Mat3x4>, 2> m_boneTrfs;
	SceneDynamicArray<Trf> m_trackPoseTrfs; // Scratch pose of a single track
	SceneDynamicArray<Trf> m_poseTrfs; // The final pose after all tracks are combined
	Aabb m_boneBoundingVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));
	Array<Track, kMaxAnimationTracks> m_tracks;

	SceneHashMap<U32, std::pair<AnimationResourcePtr, U32>> m_idToAnimationResource;

	AnimationState m_dummyState;

	// For root motion calculation:
	U32 m_rootBoneIdx = kMaxU32; // Cache it
	Vec3 m_skeletonUp = Vec3(0.0f, 1.0f, 0.0f); // The up direction in skeleton space
	Vec3 m_skeletonForward = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 m_rootRestPosition = Vec3(0.0f); // Rest translation of the root bone in skeleton space
	Quat m_rootRestRot = Quat::getIdentity();
	Vec3 m_rootMotionDelta = Vec3(0.0f);
	F32 m_rootYawDeltaAngle = 0.0f;
	RootMotionMode m_rootMotionMode = RootMotionMode::kDisabled;

	U8 m_crntBoneTrfs = 0;
	U8 m_prevBoneTrfs = 1;

	Bool m_updatedLastFrame : 1 = true;
	Bool m_resourceDirty : 1 = true;
	Bool m_boneTransformsReallocatedThisFrame : 1 = false;
#if ANKI_WITH_EDITOR
	Bool m_playOnEditor : 1 = false;
#endif

	GpuSceneBufferAllocation m_gpuSceneBoneTransforms;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;

	void visitBones(const Bone& bone, const Mat3x4& parentTrf, Vec4& minExtend, Vec4& maxExtend);

	void registerAnimationInternal(U32 animId, CString animationFilepath);

	void unregisterAnimationInternal(U32 animId);

	AnimationState& playAnimationInternal(U32 track, U32 animId, Second blendInTime, AnimationStart animStart);

	AnimationState& getAnimationStateInternal(U32 trackIdx, U32 animId);
};

} // end namespace anki
