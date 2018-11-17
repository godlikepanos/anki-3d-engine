// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/resource/Forward.h>
#include <anki/util/Forward.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Skin component.
class SkinComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::SKIN;
	static const U MAX_ANIMATION_TRACKS = 2;

	SkinComponent(SceneNode* node, SkeletonResourcePtr skeleton);

	~SkinComponent();

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

	void playAnimation(U track, AnimationResourcePtr anim, Second startTime, Bool repeat);

	const DynamicArray<Mat4>& getBoneTransforms() const
	{
		return m_boneTrfs;
	}

private:
	class Track
	{
	public:
		AnimationResourcePtr m_anim;
		F64 m_time;
		Bool8 m_repeat;
	};

	SceneNode* m_node;
	SkeletonResourcePtr m_skeleton;
	DynamicArray<Mat4> m_boneTrfs;
	Array<Track, MAX_ANIMATION_TRACKS> m_tracks;

	void visitBones(const Bone& bone, const Mat4& parentTrf, const BitSet<128, U8>& bonesAnimated);
};
/// @}

} // end namespace anki
