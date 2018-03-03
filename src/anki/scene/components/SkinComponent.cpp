// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/SkinComponent.h>
#include <anki/resource/SkeletonResource.h>
#include <anki/resource/AnimationResource.h>
#include <anki/util/BitSet.h>

namespace anki
{

SkinComponent::SkinComponent(SceneNode* node, SkeletonResourcePtr skeleton)
	: SceneComponent(CLASS_TYPE, node)
	, m_skeleton(skeleton)
{
	m_boneTrfs.create(getAllocator(), m_skeleton->getBones().getSize());
	for(Mat4& trf : m_boneTrfs)
	{
		trf.setIdentity();
	}
}

SkinComponent::~SkinComponent()
{
	m_boneTrfs.destroy(getAllocator());
}

void SkinComponent::playAnimation(U track, AnimationResourcePtr anim, F64 startTime, Bool repeat)
{
	m_tracks[track].m_anim = anim;
	m_tracks[track].m_time = startTime;
	m_tracks[track].m_repeat = repeat;
}

Error SkinComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	updated = false;
	const F64 timeDiff = crntTime - prevTime;

	for(Track& track : m_tracks)
	{
		if(!track.m_anim.isCreated())
		{
			continue;
		}

		updated = true;

		const F64 animTime = track.m_time;
		track.m_time += timeDiff;

		// Iterate the animation channels and interpolate
		BitSet<128> bonesAnimated(false);
		for(U i = 0; i < track.m_anim->getChannels().getSize(); ++i)
		{
			const AnimationChannel& channel = track.m_anim->getChannels()[i];
			const Bone* bone = m_skeleton->tryFindBone(channel.m_name.toCString());
			if(!bone)
			{
				ANKI_SCENE_LOGW("Animation is referencing unknown bone \"%s\"", &channel.m_name[0]);
				continue;
			}

			// Interpolate
			Vec3 position;
			Quat rotation;
			F32 scale;
			track.m_anim->interpolate(i, animTime, position, rotation, scale);

			// Store
			bonesAnimated.set(bone->getIndex());
			m_boneTrfs[bone->getIndex()] = Mat4(position.xyz1(), Mat3(rotation), 1.0f) * bone->getVertexTransform();
		}

		// Walk the bone hierarchy to add additional transforms
		visitBones(m_skeleton->getRootBone(), Mat4::getIdentity(), bonesAnimated);
	}

	return Error::NONE;
}

void SkinComponent::visitBones(const Bone& bone, const Mat4& parentTrf, const BitSet<128>& bonesAnimated)
{
	Mat4 myTrf = parentTrf * bone.getTransform();

	if(bonesAnimated.get(bone.getIndex()))
	{
		m_boneTrfs[bone.getIndex()] = myTrf * m_boneTrfs[bone.getIndex()];
	}
	else
	{
		m_boneTrfs[bone.getIndex()] = myTrf * bone.getVertexTransform();
	}

	for(const Bone* child : bone.getChildren())
	{
		visitBones(*child, myTrf, bonesAnimated);
	}
}

} // end namespace anki
