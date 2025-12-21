// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/SkeletonResource.h>
#include <AnKi/Resource/AnimationResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

SkinComponent::SkinComponent(SceneNode* node, U32 uuid)
	: SceneComponent(node, kClassType, uuid)
{
}

SkinComponent::~SkinComponent()
{
}

SkinComponent& SkinComponent::setSkeletonFilename(CString fname)
{
	SkeletonResourcePtr newRsrc;
	if(ResourceManager::getSingleton().loadResource(fname, newRsrc))
	{
		ANKI_SCENE_LOGE("Failed to load skeleton");
	}
	else
	{
		m_resourceDirty = !m_resource || (m_resource->getUuid() != newRsrc->getUuid());
		m_resource = std::move(newRsrc);
	}

	return *this;
}

CString SkinComponent::getSkeletonFilename() const
{
	return (m_resource) ? m_resource->getFilename() : "*Error*";
}

void SkinComponent::playAnimation(U32 trackIdx, AnimationResourcePtr anim, const AnimationPlayInfo& info)
{
	const Second animDuration = anim->getDuration();

	Track& track = m_tracks[trackIdx];

	track.m_anim = anim;
	track.m_absoluteStartTime = m_absoluteTime + info.m_startTime;
	track.m_relativeTimePassed = 0.0;
	if(info.m_repeatTimes > 0.0)
	{
		track.m_blendInTime = min(animDuration * info.m_repeatTimes, info.m_blendInTime);
		track.m_blendOutTime = min(animDuration * info.m_repeatTimes, info.m_blendOutTime);
	}
	else
	{
		track.m_blendInTime = info.m_blendInTime;
		track.m_blendOutTime = 0.0; // Irrelevant
	}
	track.m_repeatTimes = info.m_repeatTimes;
	track.m_animationSpeedScale = max(0.1f, info.m_animationSpeedScale);
}

void SkinComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	m_boneTransformsReallocatedThisFrame = false;

	if(m_resourceDirty || !isValid()) [[unlikely]]
	{
		// Cleanup
		m_boneTrfs[0].destroy();
		m_boneTrfs[1].destroy();
		m_animationTrfs.destroy();
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneBoneTransforms);

		m_boneTransformsReallocatedThisFrame = true;
	}

	if(!isValid()) [[unlikely]]
	{
		return;
	}

	// Always update two frames because of the bone transforms buffer
	const Bool updatedLastFrame = m_updatedLastFrame;
	const Bool resourceDirty = m_resourceDirty;
	m_resourceDirty = false;
	Bool animationRun = false;

	if(resourceDirty) [[unlikely]]
	{
		// Create
		const U32 boneCount = m_resource->getBones().getSize();
		m_boneTrfs[0].resize(boneCount, Mat3x4::getIdentity());
		m_boneTrfs[1].resize(boneCount, Mat3x4::getIdentity());
		m_animationTrfs.resize(boneCount, Trf{Vec3(0.0f), Quat::getIdentity(), 1.0f});

		m_gpuSceneBoneTransforms = GpuSceneBuffer::getSingleton().allocate(sizeof(Mat4) * boneCount * 2, 4);

		m_boneTransformsReallocatedThisFrame = true;
	}

	const Second dt = info.m_dt;

	Vec4 minExtend(kMaxF32, kMaxF32, kMaxF32, 0.0f);
	Vec4 maxExtend(kMinF32, kMinF32, kMinF32, 0.0f);

	BitSet<128> bonesAnimated(false);

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

		const Second clipDuration = track.m_anim->getDuration();
		const Second animationDuration = track.m_repeatTimes * clipDuration;

		if(track.m_repeatTimes > 0.0 && track.m_relativeTimePassed > animationDuration)
		{
			// Animation finished
			continue;
		}

		animationRun = true;

		const Second animTime = track.m_relativeTimePassed;
		track.m_relativeTimePassed += dt * Second(track.m_animationSpeedScale);

		// Iterate the animation channels and interpolate
		for(U32 i = 0; i < track.m_anim->getChannels().getSize(); ++i)
		{
			const AnimationChannel& channel = track.m_anim->getChannels()[i];
			const Bone* bone = m_resource->tryFindBone(channel.m_name.toCString());
			if(!bone)
			{
				ANKI_SCENE_LOGW("Animation is referencing unknown bone \"%s\"", &channel.m_name[0]);
				continue;
			}
			const U32 boneIdx = bone->getIndex();

			// Interpolate
			Vec3 position;
			Quat rotation;
			F32 scale;
			track.m_anim->interpolate(i, animTime, position, rotation, scale);

			// Blend with previous track
			if(bonesAnimated.get(boneIdx) && (track.m_blendInTime > 0.0 || track.m_blendOutTime > 0.0))
			{
				F32 blendInFactor;
				if(track.m_blendInTime > 0.0)
				{
					blendInFactor = min(1.0f, F32(animTime / track.m_blendInTime));
				}
				else
				{
					blendInFactor = 1.0f;
				}

				F32 blendOutFactor;
				if(track.m_blendOutTime > 0.0)
				{
					blendOutFactor = min(1.0f, F32((animationDuration - animTime) / track.m_blendOutTime));
				}
				else
				{
					blendOutFactor = 1.0f;
				}

				const F32 factor = blendInFactor * blendOutFactor;

				if(factor < 1.0f)
				{
					const Trf& prevTrf = m_animationTrfs[boneIdx];

					position = linearInterpolate(prevTrf.m_translation, position, factor);
					rotation = prevTrf.m_rotation.slerp(rotation, factor);
					scale = linearInterpolate(prevTrf.m_scale, scale, factor);
				}
			}

			// Store
			bonesAnimated.set(boneIdx);
			m_animationTrfs[boneIdx] = {position, rotation, scale};
		}
	}

	if(updatedLastFrame || resourceDirty || animationRun)
	{
		m_prevBoneTrfs = m_crntBoneTrfs;
		m_crntBoneTrfs = m_crntBoneTrfs ^ 1;

		// Walk the bone hierarchy to add additional transforms
		visitBones(m_resource->getRootBone(), Mat3x4::getIdentity(), bonesAnimated, minExtend, maxExtend);

		const Vec4 e(kEpsilonf, kEpsilonf, kEpsilonf, 0.0f);
		m_boneBoundingVolume.setMin(minExtend - e);
		m_boneBoundingVolume.setMax(maxExtend + e);

		// Update the GPU scene
		const U32 boneCount = m_resource->getBones().getSize();
		DynamicArray<Mat3x4, MemoryPoolPtrWrapper<StackMemoryPool>> trfs(info.m_framePool);
		trfs.resize(boneCount * 2);
		for(U32 i = 0; i < boneCount; ++i)
		{
			trfs[i * 2 + 0] = getBoneTransforms()[i];
			trfs[i * 2 + 1] = getPreviousFrameBoneTransforms()[i];
		}
		GpuSceneMicroPatcher::getSingleton().newCopy(m_gpuSceneBoneTransforms, trfs.getSizeInBytes(), trfs.getBegin());
	}
	else
	{
		m_prevBoneTrfs = m_crntBoneTrfs;
	}

	m_absoluteTime += dt;
	updated = updatedLastFrame || resourceDirty || animationRun;
	m_updatedLastFrame = resourceDirty || animationRun;
}

void SkinComponent::visitBones(const Bone& bone, const Mat3x4& parentTrf, const BitSet<128>& bonesAnimated, Vec4& minExtend, Vec4& maxExtend)
{
	Mat3x4 outMat;

	if(bonesAnimated.get(bone.getIndex()))
	{
		const Trf& t = m_animationTrfs[bone.getIndex()];
		outMat = parentTrf.combineTransformations(Mat3x4(t.m_translation.xyz, Mat3(t.m_rotation), Vec3(t.m_scale)));
	}
	else
	{
		outMat = parentTrf.combineTransformations(bone.getTransform());
	}

	m_boneTrfs[m_crntBoneTrfs][bone.getIndex()] = outMat.combineTransformations(bone.getVertexTransform());

	// Update volume
	const Vec3 bonePos = outMat * Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	minExtend = minExtend.min(bonePos.xyz0);
	maxExtend = maxExtend.max(bonePos.xyz0);

	for(const Bone* child : bone.getChildren())
	{
		visitBones(*child, outMat, bonesAnimated, minExtend, maxExtend);
	}
}

Error SkinComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);

	for(Track& track : m_tracks)
	{
		ANKI_SERIALIZE(track.m_anim, 1);
		ANKI_SERIALIZE(track.m_absoluteStartTime, 1);
		ANKI_SERIALIZE(track.m_relativeTimePassed, 1);
		ANKI_SERIALIZE(track.m_blendInTime, 1);
		ANKI_SERIALIZE(track.m_blendOutTime, 1);
		ANKI_SERIALIZE(track.m_repeatTimes, 1);
		ANKI_SERIALIZE(track.m_animationSpeedScale, 1);
	}

	return Error::kNone;
}

} // end namespace anki
