// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

namespace anki {

/// @addtogroup scene
/// @{

/// @memberof SkinComponent
class AnimationPlayInfo
{
public:
	/// The time the animation will start after being pushed in SkinComponent::playAnimation().
	Second m_startTime = 0.0;

	/// Negative means infinite.
	F32 m_repeatTimes = 1.0f;

	/// The time from when the animation starts until it fully replaces the animations of previous tracks.
	Second m_blendInTime = 0.0f;

	/// The time from when the animation ends until it until it has zero influence to the animations of previous tracks.
	Second m_blendOutTime = 0.0f;
};

/// Skin component.
class SkinComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SkinComponent)

public:
	static constexpr U32 kMaxAnimationTracks = 4;

	SkinComponent(SceneNode* node);

	~SkinComponent();

	/// Load the skeleton resource.
	Error loadSkeletonResource(CString filename);

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void playAnimation(U32 track, AnimationResourcePtr anim, const AnimationPlayInfo& info);

	ConstWeakArray<Mat4> getBoneTransforms() const
	{
		return m_boneTrfs[m_crntBoneTrfs];
	}

	ConstWeakArray<Mat4> getPreviousFrameBoneTransforms() const
	{
		return m_boneTrfs[m_prevBoneTrfs];
	}

	const SkeletonResourcePtr& getSkeleronResource() const
	{
		return m_skeleton;
	}

	const Aabb& getBoneBoundingVolumeLocalSpace() const
	{
		return m_boneBoundingVolume;
	}

	Bool isEnabled() const
	{
		return m_skeleton.isCreated();
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
	};

	class Trf
	{
	public:
		Vec3 m_translation;
		Quat m_rotation;
		F32 m_scale;
	};

	SceneNode* m_node;
	SkeletonResourcePtr m_skeleton;
	Array<DynamicArray<Mat4>, 2> m_boneTrfs;
	DynamicArray<Trf> m_animationTrfs;
	Aabb m_boneBoundingVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));
	Array<Track, kMaxAnimationTracks> m_tracks;
	Second m_absoluteTime = 0.0;
	U8 m_crntBoneTrfs = 0;
	U8 m_prevBoneTrfs = 1;

	void visitBones(const Bone& bone, const Mat4& parentTrf, const BitSet<128, U8>& bonesAnimated, Vec4& minExtend,
					Vec4& maxExtend);
};
/// @}

} // end namespace anki
