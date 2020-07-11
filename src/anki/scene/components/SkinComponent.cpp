// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/SkinComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/resource/SkeletonResource.h>
#include <anki/resource/AnimationResource.h>
#include <anki/util/BitSet.h>

namespace anki
{

SkinComponent::SkinComponent(SceneNode* node, SkeletonResourcePtr skeleton)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
	, m_skeleton(skeleton)
{
	ANKI_ASSERT(node);

	m_boneTrfs.create(m_node->getAllocator(), m_skeleton->getBones().getSize());
	for(Mat4& trf : m_boneTrfs)
	{
		trf.setIdentity();
	}
}

SkinComponent::~SkinComponent()
{
	m_boneTrfs.destroy(m_node->getAllocator());
}

void SkinComponent::playAnimation(U32 track, AnimationResourcePtr anim, const AnimationPlayInfo& info)
{
	m_tracks[track].m_anim = anim;
	m_tracks[track].m_absoluteStartTime = m_absoluteTime + info.m_startTime;
	m_tracks[track].m_relativeTimePassed = 0.0;
	m_tracks[track].m_repeatTimes = info.m_repeatTimes;
}

Error SkinComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	ANKI_ASSERT(&node == m_node);

	updated = false;
	const Second dt = crntTime - prevTime;

	Vec4 minExtend{MAX_F32, MAX_F32, MAX_F32, 0.0f};
	Vec4 maxExtend{MIN_F32, MIN_F32, MIN_F32, 0.0f};

	BitSet<128> bonesAnimated{false};

	for(Track& track : m_tracks)
	{
		if(!track.m_anim.isCreated())
		{
			continue;
		}

		if(track.m_absoluteStartTime > m_absoluteTime)
		{
			// Hasn't started yet
			continue;
		}

		const Second animationDuration = track.m_anim->getDuration();

		if(track.m_repeatTimes > 0.0 && track.m_relativeTimePassed > track.m_repeatTimes * animationDuration)
		{
			// Animation finished
			continue;
		}

		updated = true;

		const Second animTime = track.m_relativeTimePassed;
		track.m_relativeTimePassed += dt;

		// Iterate the animation channels and interpolate
		for(U32 i = 0; i < track.m_anim->getChannels().getSize(); ++i)
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
			m_boneTrfs[bone->getIndex()] = Mat4(position.xyz1(), Mat3(rotation), scale);
		}
	}

	// Always update the 1st time
	updated = updated || (m_absoluteTime == 0.0);

	if(updated)
	{
		// Walk the bone hierarchy to add additional transforms
		visitBones(m_skeleton->getRootBone(), Mat4::getIdentity(), bonesAnimated, minExtend, maxExtend);

		const Vec4 E{EPSILON, EPSILON, EPSILON, 0.0f};
		m_boneBoundingVolume.setMin(minExtend - E);
		m_boneBoundingVolume.setMax(maxExtend + E);
	}

	m_absoluteTime += dt;

	return Error::NONE;
}

void SkinComponent::visitBones(
	const Bone& bone, const Mat4& parentTrf, const BitSet<128>& bonesAnimated, Vec4& minExtend, Vec4& maxExtend)
{
	Mat4 outMat;

	if(bonesAnimated.get(bone.getIndex()))
	{
		outMat = parentTrf * m_boneTrfs[bone.getIndex()];
	}
	else
	{
		outMat = parentTrf * bone.getTransform();
	}

	m_boneTrfs[bone.getIndex()] = outMat * bone.getVertexTransform();

	// Update volume
	const Vec4 bonePos = outMat * Vec4{0.0f, 0.0f, 0.0f, 1.0f};
	minExtend = minExtend.min(bonePos.xyz0());
	maxExtend = maxExtend.max(bonePos.xyz0());

	for(const Bone* child : bone.getChildren())
	{
		visitBones(*child, outMat, bonesAnimated, minExtend, maxExtend);
	}
}

} // end namespace anki
