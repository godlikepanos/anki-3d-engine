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
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

SkinComponent::SkinComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
	m_node = init.m_node;

	// Bake the world transform into the local or ignoring the parent will snap the node to its parent-relative transform
	if(m_node->getParent())
	{
		m_node->setLocalTransform(m_node->getWorldTransform());
	}

	m_node->setIgnoreParentTransform(true);
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

AnimationState& SkinComponent::playAnimationInternal(U32 trackIdx, U32 animId, Second blendInTime, AnimationStart animStart)
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
			Animation& discarded = track.m_anims[track.m_lastAnimationIdx % kMaxAnimationsPerTrack];
			if(discarded.m_started)
			{
				m_pendingAnimationsEnded.emplaceBack(std::make_pair(trackIdx, discarded.m_animId));
			}

			discarded = {};
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
	anim.m_interpolationAnimTime = kMaxSecond;
	anim.m_rootPosition = Vec3(0.0f);
	anim.m_rootRotation = Quat::getIdentity();
	anim.m_started = false;

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

	for(auto pair : m_animationsStarted)
	{
		info.m_node->onAnimationStarted(pair.first, pair.second);
	}
	for(auto pair : m_animationsEnded)
	{
		info.m_node->onAnimationEnded(pair.first, pair.second);
	}
	m_animationsStarted.destroy();
	m_animationsEnded.destroy();

	if(m_resourceDirty || !m_resource) [[unlikely]]
	{
		// Cleanup
		m_boneTrfs[0].destroy();
		m_boneTrfs[1].destroy();
		m_poseTrfs[0].destroy();
		m_poseTrfs[1].destroy();
		m_trackPoseTrfs.destroy();
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneBoneTransforms);
		m_ragdoll.reset(nullptr);

		m_boneTransformsReallocatedThisFrame = true;
	}

	if(!m_resource) [[unlikely]]
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
		m_poseTrfs[0].resize(boneCount, Trf{Vec3(0.0f), Quat::getIdentity(), 1.0f});
		m_poseTrfs[1].resize(boneCount, Trf{Vec3(0.0f), Quat::getIdentity(), 1.0f});
		m_trackPoseTrfs.resize(boneCount, Trf{Vec3(0.0f), Quat::getIdentity(), 1.0f});

		m_gpuSceneBoneTransforms = GpuSceneBuffer::getSingleton().allocate(sizeof(Mat4) * boneCount * 2, 4);

		m_boneTransformsReallocatedThisFrame = true;

		// Cache some root bone info for root motion
		const Bone& rootBone = m_resource->getRootBone();
		m_rootBoneIdx = rootBone.getIndex();
		m_rootRestPosition = rootBone.getTransform().getTranslationPart();
		m_rootRestRot = Quat(rootBone.getTransform().getRotationPart());

		// The mesh is Y-up and restTrf * vertexTrf maps mesh space to skeleton space
		const Mat3 meshToSkeleton = rootBone.getTransform().combineTransformations(rootBone.getVertexTransform()).getRotationPart();
		m_skeletonUp = (meshToSkeleton * Vec3(0.0f, 1.0f, 0.0f)).normalize();
		m_skeletonForward = (meshToSkeleton * Vec3(0.0f, 0.0f, 1.0f)).normalize();
	}

#if ANKI_WITH_EDITOR
	const Bool paused = info.m_paused && !m_playOnEditor;
#else
	const Bool paused = info.m_paused;
#endif

	const Second dt = (paused) ? 0.0 : info.m_dt;

	Vec4 minExtend(kMaxF32, kMaxF32, kMaxF32, 0.0f);
	Vec4 maxExtend(kMinF32, kMinF32, kMinF32, 0.0f);

	// The pose is computed from scratch every frame. Keep the previous one for the ragdoll
	m_prevPoseTrfs = m_crntPoseTrfs;
	m_crntPoseTrfs = m_crntPoseTrfs ^ 1;

	for(U32 boneIdx = 0; boneIdx < m_poseTrfs[m_crntPoseTrfs].getSize(); ++boneIdx)
	{
		m_poseTrfs[m_crntPoseTrfs][boneIdx].m_translation = m_resource->getBones()[boneIdx].getTransform().getTranslationPart();
		m_poseTrfs[m_crntPoseTrfs][boneIdx].m_rotation = Quat(m_resource->getBones()[boneIdx].getTransform().getRotationPart());
		const Vec3 scale = m_resource->getBones()[boneIdx].getTransform().getRotationPart().extractScale();
		m_poseTrfs[m_crntPoseTrfs][boneIdx].m_scale =
			max(max(scale.x, scale.y), scale.z); // Not ideal but we don't support non-uniform scale in skeletons
	}

	m_rootMotionDelta = Vec3(0.0f);
	m_rootYawDeltaAngle = 0.0f;

	for(U32 trackIdx = 0; trackIdx < kMaxAnimationTracks; ++trackIdx)
	{
		Track& track = m_tracks[trackIdx];
		ANKI_ASSERT(track.m_crntAnimationIdx <= track.m_lastAnimationIdx);
		BitSet<128> trackBonesAnimated(false);

		// Gather the animations to use and blend between
		Animation* firstAnim = nullptr;
		Animation* secondAnim = nullptr;
		F32 blendInFactor = 1.0f;
		Bool clampFirstAnimAtTheEnd = false;
		F32 trackBlendFactor = 0.0f;
		for(U32 i = track.m_crntAnimationIdx; i <= track.m_lastAnimationIdx; ++i)
		{
			Animation& crntAnim = track.m_anims[i % kMaxAnimationsPerTrack];
			Animation* nextAnim = (i + 1 <= track.m_lastAnimationIdx) ? &track.m_anims[(i + 1) % kMaxAnimationsPerTrack] : nullptr;

			if(!crntAnim.m_resource)
			{
				// No animations, break the loop
				ANKI_ASSERT(nextAnim == nullptr && track.m_crntAnimationIdx == track.m_lastAnimationIdx);
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
				if(crntAnim.m_started)
				{
					m_pendingAnimationsEnded.emplaceBack(std::make_pair(trackIdx, crntAnim.m_animId));
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
				if(crntAnim.m_started)
				{
					m_pendingAnimationsEnded.emplaceBack(std::make_pair(trackIdx, crntAnim.m_animId));
				}
				continue;
			}

			if(nextAnim)
			{
				blendInFactor = min(1.0f, F32(nextAnim->m_state.m_animationTime / nextAnim->m_blendInTime));
			}

			firstAnim = &crntAnim;
			secondAnim = nextAnim;
			clampFirstAnimAtTheEnd = crntAnimFinished;

			// Compute the track blend weight
			trackBlendFactor =
				(track.m_state.m_blendInTime > 0.0) ? min(1.0f, F32(crntAnim.m_state.m_animationTime / track.m_state.m_blendInTime)) : 1.0f;

			const Animation& fadeAnim = (nextAnim) ? *nextAnim : crntAnim;
			const Second clipDuration = fadeAnim.m_resource->getDuration();
			const Second animDuration = (fadeAnim.m_state.m_repeatTimes < 0.0) ? kMaxSecond : fadeAnim.m_state.m_repeatTimes * clipDuration;
			const Second remainingTime = animDuration - fadeAnim.m_state.m_animationTime;
			const F32 blendOutFactor = min(1.0f, F32(remainingTime / track.m_state.m_blendOutTime));

			trackBlendFactor *= blendOutFactor;

			break;
		}
		ANKI_ASSERT(track.m_crntAnimationIdx <= track.m_lastAnimationIdx);

		// Interpolate and blend animations
		Array<Animation*, 2> anims = {firstAnim, secondAnim};
		Array<Vec3, 2> rootMotionDeltas = {Vec3(0.0f), Vec3(0.0f)};
		Array<Quat, 2> rootRotDeltas = {Quat::getIdentity(), Quat::getIdentity()};
		for(Animation* panim : anims)
		{
			if(!panim)
			{
				continue;
			}
			Animation& anim = *panim;

			if(!anim.m_started)
			{
				anim.m_started = true;
				m_pendingAnimationsStarted.emplaceBack(std::make_pair(trackIdx, anim.m_animId));
			}

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
			Vec3 rootPosBeforeBlend = Vec3(0.0f);
			Quat rootRotBeforeBlend = Quat::getIdentity();
			U32 rootBoneChannel = kMaxU32;
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

				if(boneIdx == m_rootBoneIdx)
				{
					rootPosBeforeBlend = position;
					rootRotBeforeBlend = rotation;
					rootBoneChannel = i;
				}

				// Blend with next animation
				if(blendInFactor < 1.0f && panim == secondAnim && trackBonesAnimated.get(boneIdx))
				{
					const Trf& prevTrf = m_trackPoseTrfs[boneIdx];

					position = linearInterpolate(prevTrf.m_translation, position, blendInFactor);
					rotation = prevTrf.m_rotation.slerp(rotation, blendInFactor);
					scale = linearInterpolate(prevTrf.m_scale, scale, blendInFactor);
				}

				// Store
				trackBonesAnimated.set(boneIdx);
				m_trackPoseTrfs[boneIdx] = {position, rotation, scale};
			}

			animationRun = true;

			// Compute root motion
			const Second prevInterpolationAnimTime = (anim.m_interpolationAnimTime != kMaxSecond) ? anim.m_interpolationAnimTime : animTime;
			const Second clipDuration = anim.m_resource->getDuration();
			const Second clipStart = anim.m_resource->getStartingTime();
			const Vec3 rootPos = rootPosBeforeBlend;
			const Quat rootRot = rootRotBeforeBlend;
			const Vec3 prevRootPos = (anim.m_interpolationAnimTime != kMaxSecond) ? anim.m_rootPosition : rootPos;
			const Quat prevRootRot = (anim.m_interpolationAnimTime != kMaxSecond) ? anim.m_rootRotation : rootRot;
			anim.m_interpolationAnimTime = animTime;
			anim.m_rootPosition = rootPos;
			anim.m_rootRotation = rootRot;
			Vec3 rootDelta = Vec3(0.0f);
			Quat rootRotDelta = Quat::getIdentity();
			if(rootBoneChannel != kMaxU32)
			{
				const Bool clamped = panim == firstAnim && clampFirstAnimAtTheEnd;

				if(clamped || floor((animTime - clipStart) / clipDuration) == floor((prevInterpolationAnimTime - clipStart) / clipDuration))
				{
					// Clip hasn't looped, our job is easy
					rootDelta = rootPos - prevRootPos;
					rootRotDelta = rootRot * prevRootRot.conjugated(); // In skeleton space, not in the root bone's space
				}
				else
				{
					// Clip looped, we need to do more work

					F32 scale;

					Vec3 posInEndOfClip;
					Quat rotInEndOfClip;
					anim.m_resource->interpolate(rootBoneChannel, clipStart + clipDuration, posInEndOfClip, rotInEndOfClip, scale);

					Vec3 posBeginningOfClip;
					Quat rotBeginningOfClip;
					anim.m_resource->interpolate(rootBoneChannel, anim.m_resource->getStartingTime(), posBeginningOfClip, rotBeginningOfClip, scale);

					rootDelta = posInEndOfClip - prevRootPos;
					rootDelta += rootPos - posBeginningOfClip;

					// From prev to the end of the clip and then from the beginning of the clip to crnt
					rootRotDelta = (rootRot * rotBeginningOfClip.conjugated()) * (rotInEndOfClip * prevRootRot.conjugated());
				}

				if(m_rootMotionMode == RootMotionMode::kHorizontalAndYaw)
				{
					// The yaw is removed from the pose and the node rotates instead so move the delta from the clip's space to the root's prev facing
					Vec3 prevForward = Mat3(prevRootRot * m_rootRestRot.conjugated()) * m_skeletonForward;
					prevForward -= m_skeletonUp * prevForward.dot(m_skeletonUp);
					const F32 prevYaw = atan2(m_skeletonForward.cross(prevForward).dot(m_skeletonUp), m_skeletonForward.dot(prevForward));
					rootDelta = Mat3(Axisang(-prevYaw, m_skeletonUp)) * rootDelta;
				}
			}
			rootMotionDeltas[(panim == firstAnim) ? 0 : 1] = rootDelta;
			rootRotDeltas[(panim == firstAnim) ? 0 : 1] = rootRotDelta;
		} // for(Animation* panim : anims)

		// Compute root motion deltas
		if(secondAnim == nullptr)
		{
			rootMotionDeltas[1] = rootMotionDeltas[0];
			rootRotDeltas[1] = rootRotDeltas[0];
		}
		Vec3 trackRootMotionDelta = linearInterpolate(rootMotionDeltas[0], rootMotionDeltas[1], blendInFactor);
		// Project trackRootMotionDelta to the plane defined by m_skeletonUp
		trackRootMotionDelta -= m_skeletonUp * trackRootMotionDelta.dot(m_skeletonUp);

		Quat trackRootRotDelta = rootRotDeltas[0].slerp(rootRotDeltas[1], blendInFactor);
		const Mat3 trackRootRotDeltaRot(trackRootRotDelta);
		Vec3 trackRootForwardDir = trackRootRotDeltaRot * m_skeletonForward;
		// Project trackRootForwardDir to the plane defined by m_skeletonUp
		trackRootForwardDir -= m_skeletonUp * trackRootForwardDir.dot(m_skeletonUp);
		trackRootForwardDir = trackRootForwardDir.normalize();
		const F32 trackYawAngle = atan2(m_skeletonForward.cross(trackRootForwardDir).dot(m_skeletonUp), m_skeletonForward.dot(trackRootForwardDir));

		// Now blend with the previous track
		if(trackBlendFactor == 0.0f)
		{
			// Nothing to do
		}
		else if(trackBlendFactor >= 1.0f && track.m_state.m_blendMode == AnimationBlendMode::kOverride)
		{
			// No need to blend
			for(U32 i = 0; i < m_poseTrfs[m_crntPoseTrfs].getSize(); ++i)
			{
				if(trackBonesAnimated.get(i))
				{
					m_poseTrfs[m_crntPoseTrfs][i] = m_trackPoseTrfs[i];
				}
			}

			if(trackBonesAnimated.get(m_rootBoneIdx))
			{
				m_rootMotionDelta = trackRootMotionDelta;
				m_rootYawDeltaAngle = trackYawAngle;
			}
		}
		else
		{
			if(track.m_state.m_blendMode == AnimationBlendMode::kOverride)
			{
				for(U32 i = 0; i < m_poseTrfs[m_crntPoseTrfs].getSize(); ++i)
				{
					if(!trackBonesAnimated.get(i))
					{
						continue;
					}

					const Trf& crnt = m_trackPoseTrfs[i];
					Trf& prev = m_poseTrfs[m_crntPoseTrfs][i];

					prev.m_translation = linearInterpolate(prev.m_translation, crnt.m_translation, trackBlendFactor);
					prev.m_rotation = prev.m_rotation.slerp(crnt.m_rotation, trackBlendFactor);
					prev.m_scale = linearInterpolate(prev.m_scale, crnt.m_scale, trackBlendFactor);
				}

				if(trackBonesAnimated.get(m_rootBoneIdx))
				{
					m_rootMotionDelta = m_rootMotionDelta.lerp(trackRootMotionDelta, trackBlendFactor);
					m_rootYawDeltaAngle = linearInterpolate(m_rootYawDeltaAngle, trackYawAngle, trackBlendFactor);
				}
			}
			else
			{
				ANKI_ASSERT(track.m_state.m_blendMode == AnimationBlendMode::kAdditive);

				for(U32 i = 0; i < m_poseTrfs[m_crntPoseTrfs].getSize(); ++i)
				{
					if(!trackBonesAnimated.get(i))
					{
						continue;
					}

					const Trf& crnt = m_trackPoseTrfs[i];
					Trf& prev = m_poseTrfs[m_crntPoseTrfs][i];

					// The additive animation is a delta from the rest pose
					const Mat3x4& restTrf = m_resource->getBones()[i].getTransform();
					const Quat restRotation(restTrf.getRotationPart());
					const Vec3 restScale3 = restTrf.getRotationPart().extractScale();
					const F32 restScale = max(max(restScale3.x, restScale3.y), restScale3.z);

					prev.m_translation += (crnt.m_translation - restTrf.getTranslationPart()) * trackBlendFactor;
					prev.m_rotation = prev.m_rotation * Quat::getIdentity().slerp(restRotation.invert() * crnt.m_rotation, trackBlendFactor);
					prev.m_scale *= linearInterpolate(1.0f, crnt.m_scale / restScale, trackBlendFactor);
				}

				if(trackBonesAnimated.get(m_rootBoneIdx))
				{
					m_rootMotionDelta += trackRootMotionDelta * trackBlendFactor;
					m_rootYawDeltaAngle += trackYawAngle * trackBlendFactor;
				}
			}
		}
	} // For all tracks

	// Adjust the pose if needed
	if(animationRun)
	{
		if(m_rootMotionMode == RootMotionMode::kHorizontal)
		{
			// The horizontal motion of the root moves the node, keep only the vertical motion
			m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_translation =
				m_rootRestPosition + m_skeletonUp * (m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_translation - m_rootRestPosition).dot(m_skeletonUp);
		}
		else if(m_rootMotionMode == RootMotionMode::kHorizontalAndYaw)
		{
			// The horizontal motion of the root moves the node, keep only the vertical motion
			m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_translation =
				m_rootRestPosition + m_skeletonUp * (m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_translation - m_rootRestPosition).dot(m_skeletonUp);

			// The node rotates with the yaw, remove only the yaw (relative to the rest pose) and keep the rest of the root's rotation
			const Quat poseRot = m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_rotation;
			Vec3 poseForward = Mat3(poseRot * m_rootRestRot.conjugated()) * m_skeletonForward;
			poseForward -= m_skeletonUp * poseForward.dot(m_skeletonUp);
			const F32 poseYaw = atan2(m_skeletonForward.cross(poseForward).dot(m_skeletonUp), m_skeletonForward.dot(poseForward));
			m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_rotation = Quat(Axisang(-poseYaw, m_skeletonUp)) * poseRot;
		}
		else if(m_rootMotionMode == RootMotionMode::kLockRoot)
		{
			ANKI_ASSERT(m_rootMotionMode == RootMotionMode::kLockRoot);
			m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_translation = m_rootRestPosition;
			m_poseTrfs[m_crntPoseTrfs][m_rootBoneIdx].m_rotation = m_rootRestRot;
		}
	}

	if(m_ragdollMode == RagdollMode::kDynamic && m_resource->getRootBone().getCapsuleRadius() == 0.0f) [[unlikely]]
	{
		ANKI_SCENE_LOGE("The skeleton has no capsules, can't create a ragdoll: %s", m_resource->getFilename().cstr());
		m_ragdollMode = RagdollMode::kDisabled;
	}

	if(m_ragdollMode == RagdollMode::kDynamic && !info.m_paused)
	{
		// The physics move the bones instead of the animations
		updateRagdoll(info, !resourceDirty);
		animationRun = true; // So the new pose gets uploaded
	}
	else
	{
		// Animations move the pose
		m_ragdoll.reset(nullptr);
	}

	if(updatedLastFrame || resourceDirty || animationRun)
	{
		m_prevBoneTrfs = m_crntBoneTrfs;
		m_crntBoneTrfs = m_crntBoneTrfs ^ 1;

		// Walk the bone hierarchy to add additional transforms
		visitBones(m_resource->getRootBone(), Mat3x4::getIdentity(), minExtend, maxExtend);

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

	// Publish the events of this update and of the playAnimation() calls since the last update
	m_animationsStarted = std::move(m_pendingAnimationsStarted);
	m_animationsEnded = std::move(m_pendingAnimationsEnded);

#if ANKI_ASSERTIONS_ENABLED
	auto hasDuplicates = [](ConstWeakArray<std::pair<U32, U32>> events) {
		for(U32 i = 0; i < events.getSize(); ++i)
		{
			for(U32 j = i + 1; j < events.getSize(); ++j)
			{
				if(events[i] == events[j])
				{
					return true;
				}
			}
		}
		return false;
	};
	ANKI_ASSERT(!hasDuplicates(m_animationsStarted) && !hasDuplicates(m_animationsEnded));
#endif
}

void SkinComponent::updateRagdoll(SceneComponentUpdateInfo& info, Bool prevPoseValid)
{
	ConstWeakArray<Bone> bones = m_resource->getBones();

	// The MoveComponent runs after us so the world transform of the node is the one of the previous frame. Compute this frame's from the local
	const Transform prevNodeTrf = m_node->getWorldTransform();
	ANKI_ASSERT(m_node->getIgnoreParentTransform() && "Set in the constructor. World and local are the same");
	const Transform nodeTrf = m_node->getLocalTransform();

	const Vec3 nodeScale = nodeTrf.getScale().xyz;
	ANKI_ASSERT(absolute(nodeScale.x - 1.0f) < 0.001f && absolute(nodeScale.y - 1.0f) < 0.001f && absolute(nodeScale.z - 1.0f) < 0.001f
				&& "Ragdolls can't be scaled. Set the scale of the node to 1");

	DynamicArray<Mat3x4, MemoryPoolPtrWrapper<StackMemoryPool>> worldTrfs(info.m_framePool);
	worldTrfs.resize(bones.getSize());

	if(!m_ragdoll)
	{
		// Create the ragdoll from the skeleton
		DynamicArray<PhysicsRagdollBone, MemoryPoolPtrWrapper<StackMemoryPool>> ragdollBones(info.m_framePool);
		ragdollBones.resize(bones.getSize());
		for(U32 i = 0; i < bones.getSize(); ++i)
		{
			const Bone& bone = bones[i];
			PhysicsRagdollBone& out = ragdollBones[i];

			out.m_name = bone.getName();
			out.m_transform = bone.getTransform();
			out.m_parent = (bone.getParent()) ? bone.getParent()->getIndex() : kMaxU32;
			out.m_capsuleRadius = bone.getCapsuleRadius();
			out.m_capsuleHalfHeight = bone.getCapsuleHalfHeight();
			out.m_capsulePosition = bone.getCapsulePosition();
			out.m_capsuleRotation = bone.getCapsuleRotation();
			out.m_constraintRotation = bone.getConstraintRotation();
			out.m_constraintParentRotation = bone.getConstraintParentRotation();
			out.m_twistMinAngle = bone.getTwistMinAngle();
			out.m_twistMaxAngle = bone.getTwistMaxAngle();
			out.m_planeAngle = bone.getPlaneAngle();
			out.m_normalAngle = bone.getNormalAngle();
		}

		PhysicsRagdollInitInfo init;
		init.m_bones = ragdollBones;
		init.m_mass = m_ragdollMass;
		init.m_contactFriction = m_ragdollFriction;
		m_ragdoll = PhysicsWorld::getSingleton().newRagdoll(init);

		// Move it to the current pose. Parents come first so their world transform is ready
		for(U32 i = 0; i < bones.getSize(); ++i)
		{
			const Trf& t = m_poseTrfs[m_crntPoseTrfs][i];
			const Mat3x4 localTrf(t.m_translation, Mat3(t.m_rotation), Vec3(t.m_scale));
			const Mat3x4& parentTrf = (bones[i].getParent()) ? worldTrfs[bones[i].getParent()->getIndex()] : Mat3x4(nodeTrf);
			worldTrfs[i] = parentTrf.combineTransformations(localTrf);
		}

		if(prevPoseValid && info.m_dt > 0.0)
		{
			// Same for the previous pose so the bodies get the velocity of the animation. Otherwise a running character stops dead and drops
			DynamicArray<Mat3x4, MemoryPoolPtrWrapper<StackMemoryPool>> prevWorldTrfs(info.m_framePool);
			prevWorldTrfs.resize(bones.getSize());
			for(U32 i = 0; i < bones.getSize(); ++i)
			{
				const Trf& t = m_poseTrfs[m_prevPoseTrfs][i];
				const Mat3x4 localTrf(t.m_translation, Mat3(t.m_rotation), Vec3(t.m_scale));
				const Mat3x4& parentTrf = (bones[i].getParent()) ? prevWorldTrfs[bones[i].getParent()->getIndex()] : Mat3x4(prevNodeTrf);
				prevWorldTrfs[i] = parentTrf.combineTransformations(localTrf);
			}

			m_ragdoll->setBoneTransforms(prevWorldTrfs, worldTrfs, info.m_dt);
		}
		else
		{
			m_ragdoll->setBoneTransforms(worldTrfs);
		}
	}

	m_ragdoll->getBoneTransforms(WeakArray<Mat3x4>(worldTrfs));

	// Move the root bone body's position to model space
	const Vec3 rootPos = Mat3x4(nodeTrf).invertTransformation() * worldTrfs[m_rootBoneIdx].getTranslationPart().xyz1;
	m_rootMotionDelta = rootPos - m_rootRestPosition;
	m_rootMotionDelta -= m_skeletonUp * m_rootMotionDelta.dot(m_skeletonUp);
	m_rootYawDeltaAngle = 0.0f; // A tumbling body has no meaningful yaw

	// Turn the bodies back to transforms relative to the parent bone, that's what the skinning expects
	for(U32 i = 0; i < bones.getSize(); ++i)
	{
		const Mat3x4& parentTrf = (bones[i].getParent()) ? worldTrfs[bones[i].getParent()->getIndex()] : Mat3x4(nodeTrf);
		const Mat3x4 localTrf = parentTrf.invertTransformation().combineTransformations(worldTrfs[i]);

		m_poseTrfs[m_crntPoseTrfs][i].m_translation = localTrf.getTranslationPart();
		m_poseTrfs[m_crntPoseTrfs][i].m_rotation = Quat(localTrf.getRotationPart());
		m_poseTrfs[m_crntPoseTrfs][i].m_scale = 1.0f;
	}
}

void SkinComponent::visitBones(const Bone& bone, const Mat3x4& parentTrf, Vec4& minExtend, Vec4& maxExtend)
{
	Mat3x4 outMat;

	const Trf& t = m_poseTrfs[m_crntPoseTrfs][bone.getIndex()];
	outMat = parentTrf.combineTransformations(Mat3x4(t.m_translation.xyz, Mat3(t.m_rotation), Vec3(t.m_scale)));

	m_boneTrfs[m_crntBoneTrfs][bone.getIndex()] = outMat.combineTransformations(bone.getVertexTransform());

	// Update volume
	const Vec3 bonePos = outMat * Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	minExtend = minExtend.min(bonePos.xyz0);
	maxExtend = maxExtend.max(bonePos.xyz0);

	for(const Bone* child : bone.getChildren())
	{
		visitBones(*child, outMat, minExtend, maxExtend);
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
		ANKI_SERIALIZE(track.m_state.m_blendMode, 1);
		ANKI_SERIALIZE(track.m_state.m_blendInTime, 1);
		ANKI_SERIALIZE(track.m_state.m_blendOutTime, 1);

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

	ANKI_SERIALIZE(m_rootMotionMode, 1);
	ANKI_SERIALIZE(m_ragdollMode, 1);
	ANKI_SERIALIZE(m_ragdollMass, 1);
	ANKI_SERIALIZE(m_ragdollFriction, 1);

	return Error::kNone;
}

AnimationState& SkinComponent::getAnimationStateInternal(U32 trackIdx, U32 animId)
{
	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		trackIdx = 0;
	}

	Track& track = m_tracks[trackIdx];
	AnimationState* state = nullptr;
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
