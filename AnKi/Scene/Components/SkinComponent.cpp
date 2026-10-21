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

SkinComponent::SkinComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
	m_node = init.m_node;
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

void SkinComponent::registerAnimationInternal(U32 animId, CString animationFilepath)
{
	AnimationResourcePtr rsrc;
	if(ResourceManager::getSingleton().loadResource(animationFilepath, rsrc))
	{
		ANKI_SCENE_LOGE("Failed to load animation: %s", animationFilepath.cstr());
	}
	else
	{
		m_idToAnimationResource.emplace(animId, std::make_pair(rsrc, animId));
	}
}

void SkinComponent::unregisterAnimationInternal(U32 animId)
{
	auto it = m_idToAnimationResource.find(animId);
	if(it == m_idToAnimationResource.getEnd())
	{
		ANKI_SCENE_LOGE("Failed to unregister animation: %u", animId);
	}
	else
	{
		m_idToAnimationResource.erase(it);
	}
}

CString SkinComponent::getSkeletonFilename() const
{
	return (ANKI_EXPECT(!!m_resource)) ? m_resource->getFilename() : "*Error*";
}

SkinComponentAnimationState& SkinComponent::playAnimationInternal(U32 trackIdx, U32 animId, Second blendInTime, AnimationStart animStart)
{
	auto rsrcIt = m_idToAnimationResource.find(animId);
	if(rsrcIt == m_idToAnimationResource.getEnd())
	{
		ANKI_SCENE_LOGE("Animation ID not found: %u", animId);
		return m_dummyState;
	}

	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		trackIdx = 0;
	}

	Track& track = m_tracks[trackIdx];

	if(animStart == AnimationStart::kNow)
	{
		// Discard the queued animations so the new one follows the current immediately
		while(track.m_lastAnimationIdx > track.m_crntAnimationIdx)
		{
			track.m_anims[track.m_lastAnimationIdx % kMaxAnimationsPerTrack] = {};
			--track.m_lastAnimationIdx;
		}
	}

	Animation& anim = track.m_anims[(track.m_lastAnimationIdx + 1) % kMaxAnimationsPerTrack];
	if(anim.m_resource)
	{
		ANKI_SCENE_LOGE("Can't queue more than %u animations", kMaxAnimationsPerTrack);
		return m_dummyState;
	}

	anim.m_resource = rsrcIt->first;
	anim.m_blendInTime = blendInTime;
	anim.m_start = animStart;
	anim.m_state = {};
	anim.m_animId = animId;

	++track.m_lastAnimationIdx;

	if(!track.m_anims[track.m_crntAnimationIdx % kMaxAnimationsPerTrack].m_resource)
	{
		// There isn't a current animation, last should be equal to current
		++track.m_crntAnimationIdx;
		ANKI_ASSERT(track.m_lastAnimationIdx == track.m_crntAnimationIdx);
	}

	return anim.m_state;
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

#if ANKI_WITH_EDITOR
	const Bool paused = info.m_paused && !m_playOnEditor;
#else
	const Bool paused = info.m_paused;
#endif

	const Second dt = (paused) ? 0.0 : info.m_dt;

	Vec4 minExtend(kMaxF32, kMaxF32, kMaxF32, 0.0f);
	Vec4 maxExtend(kMinF32, kMinF32, kMinF32, 0.0f);

	BitSet<128> bonesAnimated(false);

	for(Track& track : m_tracks)
	{
		ANKI_ASSERT(track.m_crntAnimationIdx <= track.m_lastAnimationIdx);
		BitSet<128> trackBonesAnimated(false);

		// Gather the animations to use and blend between
		Animation* firstAnim = nullptr;
		Animation* secondAnim = nullptr;
		F32 blendInFactor = 1.0f;
		Bool clampFirstAnimAtTheEnd = false;
		for(U32 i = track.m_crntAnimationIdx; i <= track.m_lastAnimationIdx; ++i)
		{
			Animation& crntAnim = track.m_anims[i % kMaxAnimationsPerTrack];
			Animation* nextAnim = (i + 1 <= track.m_lastAnimationIdx) ? &track.m_anims[(i + 1) % kMaxAnimationsPerTrack] : nullptr;

			if(!crntAnim.m_resource)
			{
				// No animations, break the loop
				ANKI_ASSERT(nextAnim == nullptr);
				break;
			}

			if(nextAnim)
			{
				ANKI_ASSERT(nextAnim->m_resource);
			}

			const Second crntClipDuration = crntAnim.m_resource->getDuration();

			if(nextAnim && crntAnim.m_state.m_repeatTimes < 0.0f)
			{
				// If there is a next animation queued we can't have the current animation run forever

				crntAnim.m_state.m_repeatTimes = max(1.0f, std::ceil(F32(crntAnim.m_state.m_animationTime / crntClipDuration)));

				const Second timeRemaining = crntClipDuration * crntAnim.m_state.m_repeatTimes - crntAnim.m_state.m_animationTime;
				if(timeRemaining < nextAnim->m_blendInTime)
				{
					// Not enough time to blend the next animation, we need more repeat cycles

					const Second diff = nextAnim->m_blendInTime - timeRemaining;
					const F32 extraTimes = std::ceil(F32(diff / crntClipDuration));
					ANKI_ASSERT(extraTimes >= 1.0f);
					crntAnim.m_state.m_repeatTimes += extraTimes;
				}
			}

			const Second crntAnimationDuration = crntAnim.m_state.m_repeatTimes * crntClipDuration;

			const Bool crntAnimFinished = crntAnim.m_state.m_repeatTimes > 0.0 && crntAnim.m_state.m_animationTime > crntAnimationDuration;
			if(crntAnimFinished && (!nextAnim || nextAnim->m_state.m_animationTime > nextAnim->m_blendInTime))
			{
				// Current anim
				// - doesn't repeat to infinity
				// - its time exceeds the duration of the animation
				// - and the next animation has finish blending
				// then finish it

				crntAnim.m_resource.reset(nullptr);
				if(nextAnim)
				{
					++track.m_crntAnimationIdx;
				}
				continue;
			}

			const Second crntTimeRemaining = crntAnimationDuration - crntAnim.m_state.m_animationTime;
			if(nextAnim && nextAnim->m_start == AnimationStart::kQueued && crntTimeRemaining > nextAnim->m_blendInTime)
			{
				// The next animation is not scheduled yet, stop considering it
				nextAnim = nullptr;
			}

			if(nextAnim && nextAnim->m_state.m_animationTime >= nextAnim->m_blendInTime)
			{
				// Next animation can fully overtake the current one, forget the current
				crntAnim.m_resource.reset(nullptr);
				++track.m_crntAnimationIdx;
				continue;
			}

			if(nextAnim)
			{
				blendInFactor = min(1.0f, F32(nextAnim->m_state.m_animationTime / nextAnim->m_blendInTime));
			}

			firstAnim = &crntAnim;
			secondAnim = nextAnim;
			clampFirstAnimAtTheEnd = crntAnimFinished;

			break;
		}
		ANKI_ASSERT(track.m_crntAnimationIdx <= track.m_lastAnimationIdx);

		// Interpolate and blend
		Array<Animation*, 2> anims = {firstAnim, secondAnim};
		for(Animation* panim : anims)
		{
			if(!panim)
			{
				continue;
			}
			Animation& anim = *panim;

			Second animTime = anim.m_resource->getStartingTime();
			if(panim == firstAnim && clampFirstAnimAtTheEnd)
			{
				// We are keeping it alive just for blending, clamp the pose to the end
				animTime += anim.m_resource->getDuration();
			}
			else
			{
				animTime += anim.m_state.m_animationTime + max(0.0, anim.m_state.m_animationTimeBias);
			}

			anim.m_state.m_animationTime += dt * Second(anim.m_state.m_animationSpeedScale);

			// Iterate the animation channels and interpolate
			for(U32 i = 0; i < anim.m_resource->getChannels().getSize(); ++i)
			{
				const AnimationChannel& channel = anim.m_resource->getChannels()[i];
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
				anim.m_resource->interpolate(i, animTime, position, rotation, scale);

				// Blend with next animation
				if(blendInFactor < 1.0f && panim == secondAnim && trackBonesAnimated.get(boneIdx))
				{
					const Trf& prevTrf = m_animationTrfs[boneIdx];

					position = linearInterpolate(prevTrf.m_translation, position, blendInFactor);
					rotation = prevTrf.m_rotation.slerp(rotation, blendInFactor);
					scale = linearInterpolate(prevTrf.m_scale, scale, blendInFactor);
				}

				// Store
				trackBonesAnimated.set(boneIdx);
				bonesAnimated.set(boneIdx);
				m_animationTrfs[boneIdx] = {position, rotation, scale};
			}

			animationRun = true;
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

	U32 idCount = U32(m_idToAnimationResource.getSize());
	ANKI_SERIALIZE(idCount, 1);
	if(serializer.isInReadMode())
	{
		for(U32 i = 0; i < idCount; ++i)
		{
			AnimationResourcePtr rsrc;
			ANKI_SERIALIZE(rsrc, 1);
			U32 id;
			ANKI_SERIALIZE(id, 1);
			m_idToAnimationResource.emplace(id, std::make_pair(rsrc, id));
		}
	}
	else
	{
		auto it = m_idToAnimationResource.getBegin();
		for(; it != m_idToAnimationResource.getEnd(); ++it)
		{
			AnimationResourcePtr rsrc = it->first;
			ANKI_SERIALIZE(rsrc, 1);
			U32 id = it->second;
			ANKI_SERIALIZE(id, 1);
		}
	}

	for(Track& track : m_tracks)
	{
		ANKI_SERIALIZE(track.m_crntAnimationIdx, 1);
		ANKI_SERIALIZE(track.m_lastAnimationIdx, 1);

		for(Animation& anim : track.m_anims)
		{
			ANKI_SERIALIZE(anim.m_resource, 1);
			ANKI_SERIALIZE(anim.m_blendInTime, 1);
			ANKI_SERIALIZE(anim.m_start, 1);
			ANKI_SERIALIZE(anim.m_animId, 1);
			ANKI_SERIALIZE(anim.m_state.m_repeatTimes, 1);
			ANKI_SERIALIZE(anim.m_state.m_animationSpeedScale, 1);
			ANKI_SERIALIZE(anim.m_state.m_animationTime, 1);
			ANKI_SERIALIZE(anim.m_state.m_animationTimeBias, 1);
		}
	}

	return Error::kNone;
}

SkinComponentAnimationState& SkinComponent::getAnimationStateInternal(U32 trackIdx, U32 animId)
{
	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		trackIdx = 0;
	}

	Track& track = m_tracks[trackIdx];
	SkinComponentAnimationState* state = nullptr;
	for(U32 i = track.m_crntAnimationIdx; i <= track.m_lastAnimationIdx; ++i)
	{
		Animation& anim = track.m_anims[i % kMaxAnimationsPerTrack];
		if(anim.m_resource && anim.m_animId == animId)
		{
			state = &anim.m_state;
			break;
		}
	}

	if(!state)
	{
		ANKI_SCENE_LOGE("Couldn't find animation with ID: %u", animId);
		state = &m_dummyState;
	}

	return *state;
}

} // end namespace anki
