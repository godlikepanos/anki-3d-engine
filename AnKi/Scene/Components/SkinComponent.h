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

// Passed to SkinComponent::playAnimation
class AnimationPlayInfo
{
public:
	// The time the animation will start after being pushed in SkinComponent::playAnimation()
	Second m_startTime = 0.0;

	// Negative means infinite
	F32 m_repeatTimes = 1.0f;

	// The time from when the animation starts until it fully replaces the animations of previous tracks
	Second m_blendInTime = 0.0f;

	// The time from when the animation ends until it until it has zero influence to the animations of previous tracks
	Second m_blendOutTime = 0.0f;

	// For example a value of 2.0 will play the animation in double speed
	F32 m_animationSpeedScale = 1.0f;
};

// Skin component
class SkinComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SkinComponent)

public:
	static constexpr U32 kMaxAnimationTracks = 4;

	SkinComponent(const SceneComponentInitInfo& init);

	~SkinComponent();

	SkinComponent& setSkeletonFilename(CString filename);

	CString getSkeletonFilename() const;

	Bool hasSkeletonResource() const
	{
		return !!m_resource;
	}

	void playAnimation(U32 track, AnimationResourcePtr anim, const AnimationPlayInfo& info);

	Bool isValid() const
	{
		return m_resource.isCreated();
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

	ANKI_INTERNAL const SkeletonResourcePtr& getSkeleronResource() const
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

private:
	class Track
	{
	public:
		AnimationResourcePtr m_anim;
		Second m_absoluteStartTime = 0.0;
		Second m_relativeTimePassed = 0.0;
		Second m_blendInTime = 0.0;
		Second m_blendOutTime = 0.0f;
		F32 m_repeatTimes = 1.0f;
		F32 m_animationSpeedScale = 1.0f;
	};

	class Trf
	{
	public:
		Vec3 m_translation;
		Quat m_rotation;
		F32 m_scale;
	};

	SkeletonResourcePtr m_resource;
	Array<SceneDynamicArray<Mat3x4>, 2> m_boneTrfs;
	SceneDynamicArray<Trf> m_animationTrfs;
	Aabb m_boneBoundingVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));
	Array<Track, kMaxAnimationTracks> m_tracks;
	Second m_absoluteTime = 0.0;
	U8 m_crntBoneTrfs = 0;
	U8 m_prevBoneTrfs = 1;

	Bool m_updatedLastFrame : 1 = true;
	Bool m_resourceDirty : 1 = true;
	Bool m_boneTransformsReallocatedThisFrame : 1 = false;

	GpuSceneBufferAllocation m_gpuSceneBoneTransforms;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;

	void visitBones(const Bone& bone, const Mat3x4& parentTrf, const BitSet<128, U8>& bonesAnimated, Vec4& minExtend, Vec4& maxExtend);
};

} // end namespace anki
