// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/AnimationComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/AnimationResource.h>

namespace anki {

AnimationComponent::AnimationComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
}

AnimationComponent::~AnimationComponent()
{
}

void AnimationComponent::registerAnimationInternal(U32 animId, CString animationFilepath)
{
	AnimationResourcePtr rsrc;
	if(ResourceManager::getSingleton().loadResource(animationFilepath, rsrc))
	{
		ANKI_SCENE_LOGE("Failed to load animation: {}", animationFilepath);
	}
	else
	{
		m_idToAnimationResource.emplace(animId, std::make_pair(rsrc, animId));
	}
}

void AnimationComponent::unregisterAnimationInternal(U32 animId)
{
	auto it = m_idToAnimationResource.find(animId);
	if(it == m_idToAnimationResource.getEnd())
	{
		ANKI_SCENE_LOGE("Failed to unregister animation: {}", animId);
	}
	else
	{
		m_idToAnimationResource.erase(it);
	}
}

AnimationState& AnimationComponent::playAnimationInternal(U32 trackIdx, U32 animId, CString channelName, Second blendInTime, AnimationStart animStart)
{
	auto rsrcIt = m_idToAnimationResource.find(animId);
	if(rsrcIt == m_idToAnimationResource.getEnd())
	{
		ANKI_SCENE_LOGE("Animation ID not found: {}", animId);
		return m_dummyState;
	}

	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		trackIdx = kMaxAnimationTracks - 1;
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
		ANKI_SCENE_LOGE("Can't queue more than {} animations", kMaxAnimationsPerTrack);
		return m_dummyState;
	}

	U32 channelIdx;
	if(!channelName.isEmpty())
	{
		channelIdx = kMaxU32;
		for(U32 i = 0; i < rsrcIt->first->getChannels().getSize(); ++i)
		{
			if(rsrcIt->first->getChannels()[i].m_name == channelName)
			{
				channelIdx = i;
				break;
			}
		}

		if(channelIdx == kMaxU32)
		{
			ANKI_SCENE_LOGE("Channel \"{}\" not found in animation: {}", channelName, rsrcIt->first->getFilename());
			return m_dummyState;
		}
	}
	else
	{
		channelIdx = 0;
	}

	anim.m_channelIdx = channelIdx;
	anim.m_resource = rsrcIt->first;
	anim.m_blendInTime = blendInTime;
	anim.m_start = animStart;
	anim.m_state = {};
	anim.m_animId = animId;
	anim.m_interpolationAnimTime = kMaxSecond;
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

AnimationState& AnimationComponent::getAnimationStateInternal(U32 trackIdx, U32 animId)
{
	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		trackIdx = kMaxAnimationTracks - 1;
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
		ANKI_SCENE_LOGE("Couldn't find animation with ID: {}", animId);
		state = &m_dummyState;
	}

	return *state;
}

void AnimationComponent::resetTrack(U32 trackIdx)
{
	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		return;
	}

	Track& track = m_tracks[trackIdx];
	for(U32 i = track.m_crntAnimationIdx; i <= track.m_lastAnimationIdx; ++i)
	{
		const Animation& anim = track.m_anims[i % kMaxAnimationsPerTrack];
		if(anim.m_resource && anim.m_started)
		{
			m_pendingAnimationsEnded.emplaceBack(std::make_pair(trackIdx, anim.m_animId));
		}
	}

	track = {};
}

CString AnimationComponent::getFilepathOfFirstAnimationPlaying(U32 trackIdx) const
{
	if(!ANKI_EXPECT(trackIdx < kMaxAnimationTracks))
	{
		trackIdx = kMaxAnimationTracks - 1;
	}

	const Track& track = m_tracks[trackIdx];
	const Animation& anim = track.m_anims[track.m_crntAnimationIdx % kMaxAnimationsPerTrack];
	return (anim.m_resource) ? anim.m_resource->getFilename() : CString();
}

void AnimationComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	// Do callbacks
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

#if ANKI_WITH_EDITOR
	const Bool paused = info.m_paused && !m_playOnEditor;
#else
	const Bool paused = info.m_paused;
#endif
	const Second dt = (paused) ? 0.0 : info.m_dt;

	m_nodeTransformDelta = Transform::getIdentity();

	// Start animating
	Trf finalTrf;
	Trf finalDiffTrf;
	Bool isFirstTrack = true;
	for(U32 trackIdx = 0; trackIdx < kMaxAnimationTracks; ++trackIdx)
	{
		Track& track = m_tracks[trackIdx];
		ANKI_ASSERT(track.m_crntAnimationIdx <= track.m_lastAnimationIdx);

		// Gather the animations to use and blend between
		Animation* firstAnim = nullptr;
		Animation* secondAnim = nullptr;
		Bool clampFirstAnimAtTheEnd = false;
		F32 blendInFactor = 1.0f; // Blend of animations of the current track
		F32 trackBlendFactor = 0.0f; // Blend with previous track
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

			if(isFirstTrack)
			{
				// First track will always have full blending
				isFirstTrack = false;
				trackBlendFactor = 1.0f;
			}

			break;
		}

		// Interpolate and blend animations of the track
		Array<Animation*, 2> anims = {firstAnim, secondAnim};
		Trf trackTrf;
		Trf trackDiffTrf;
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

			// Interpolate
			Trf trf;
			anim.m_resource->interpolate(anim.m_channelIdx, animTime, trf.m_pos, trf.m_rot, trf.m_scale);

			// Compute the diff from previous frame
			const Second prevInterpolationAnimTime = (anim.m_interpolationAnimTime != kMaxSecond) ? anim.m_interpolationAnimTime : animTime;
			const Second clipDuration = anim.m_resource->getDuration();
			const Second clipStart = anim.m_resource->getStartingTime();
			const Bool clamped = panim == firstAnim && clampFirstAnimAtTheEnd;
			const Trf prevTrf = (anim.m_interpolationAnimTime != kMaxSecond) ? anim.m_animTrf : trf;
			anim.m_interpolationAnimTime = animTime;
			anim.m_animTrf = trf;
			Trf diffTrf;
			if(clamped || floor((animTime - clipStart) / clipDuration) == floor((prevInterpolationAnimTime - clipStart) / clipDuration))
			{
				// Clip hasn't looped, our job is easy
				diffTrf.m_pos = trf.m_pos - prevTrf.m_pos;
				diffTrf.m_rot = trf.m_rot * prevTrf.m_rot.conjugated();
				diffTrf.m_scale = trf.m_scale / prevTrf.m_scale;
			}
			else
			{
				// Clip looped, we need to do more work

				Vec3 posInEndOfClip;
				Quat rotInEndOfClip;
				F32 scaleInEndOfClip;
				anim.m_resource->interpolate(anim.m_channelIdx, clipStart + clipDuration, posInEndOfClip, rotInEndOfClip, scaleInEndOfClip);

				Vec3 posBeginningOfClip;
				Quat rotBeginningOfClip;
				F32 scaleBeginningOfClip;
				anim.m_resource->interpolate(anim.m_channelIdx, anim.m_resource->getStartingTime(), posBeginningOfClip, rotBeginningOfClip,
											 scaleBeginningOfClip);

				diffTrf.m_pos = posInEndOfClip - prevTrf.m_pos;
				diffTrf.m_pos += trf.m_pos - posBeginningOfClip;

				// From prev to the end of the clip and then from the beginning of the clip to crnt
				diffTrf.m_rot = (trf.m_rot * rotBeginningOfClip.conjugated()) * (rotInEndOfClip * prevTrf.m_rot.conjugated());

				diffTrf.m_scale = (scaleInEndOfClip / prevTrf.m_scale) * (trf.m_scale / scaleBeginningOfClip);
			}

			// Blend track animations and diffs
			if(blendInFactor < 1.0f && panim == secondAnim)
			{
				// Blend the next animation
				trackTrf.m_pos = linearInterpolate(trackTrf.m_pos, trf.m_pos, blendInFactor);
				trackTrf.m_rot = trackTrf.m_rot.slerp(trf.m_rot, blendInFactor);
				trackTrf.m_scale = linearInterpolate(trackTrf.m_scale, trf.m_scale, blendInFactor);

				trackDiffTrf.m_pos = linearInterpolate(trackDiffTrf.m_pos, diffTrf.m_pos, blendInFactor);
				trackDiffTrf.m_rot = trackDiffTrf.m_rot.slerp(diffTrf.m_rot, blendInFactor);
				trackDiffTrf.m_scale = linearInterpolate(trackDiffTrf.m_scale, diffTrf.m_scale, blendInFactor);
			}
			else
			{
				trackTrf = trf;

				trackDiffTrf = diffTrf;
			}
		} // for(Animation* panim : anims)

		// Now blend with the previous track
		if(trackBlendFactor == 0.0f)
		{
			// Nothing to do
		}
		else if(trackBlendFactor >= 1.0f && track.m_state.m_blendMode == AnimationBlendMode::kOverride)
		{
			// No need to blend

			finalTrf = trackTrf;
			finalDiffTrf = trackDiffTrf;

			updated = true;
		}
		else if(track.m_state.m_blendMode == AnimationBlendMode::kOverride)
		{
			// Need to blend
			finalTrf.m_pos = linearInterpolate(finalTrf.m_pos, trackTrf.m_pos, trackBlendFactor);
			finalTrf.m_rot = finalTrf.m_rot.slerp(trackTrf.m_rot, trackBlendFactor);
			finalTrf.m_scale = linearInterpolate(finalTrf.m_scale, trackTrf.m_scale, trackBlendFactor);

			finalDiffTrf.m_pos = linearInterpolate(finalDiffTrf.m_pos, trackDiffTrf.m_pos, trackBlendFactor);
			finalDiffTrf.m_rot = finalDiffTrf.m_rot.slerp(trackDiffTrf.m_rot, trackBlendFactor);
			finalDiffTrf.m_scale = linearInterpolate(finalDiffTrf.m_scale, trackDiffTrf.m_scale, trackBlendFactor);

			updated = true;
		}
		else
		{
			// Need to blend
			ANKI_ASSERT(track.m_state.m_blendMode == AnimationBlendMode::kAdditive);

			finalTrf.m_pos += trackTrf.m_pos * trackBlendFactor;
			finalTrf.m_rot = finalTrf.m_rot * Quat::getIdentity().slerp(trackTrf.m_rot, trackBlendFactor);
			finalTrf.m_scale *= linearInterpolate(1.0f, trackTrf.m_scale, trackBlendFactor);

			finalDiffTrf.m_pos += trackDiffTrf.m_pos * trackBlendFactor;
			finalDiffTrf.m_rot = finalDiffTrf.m_rot * Quat::getIdentity().slerp(trackDiffTrf.m_rot, trackBlendFactor);
			finalDiffTrf.m_scale *= linearInterpolate(1.0f, trackDiffTrf.m_scale, trackBlendFactor);

			updated = true;
		}
	}

	// Time to do something with the animation
	if(updated)
	{
		if(m_motionMode == NodeMotionMode::kApplyToNode)
		{
			info.m_node->setLocalTransform(Transform(finalTrf.m_pos, Mat3(finalTrf.m_rot), Vec3(finalTrf.m_scale)));
		}
		else
		{
			ANKI_ASSERT(m_motionMode == NodeMotionMode::kExtract);
			m_nodeTransformDelta = Transform(finalDiffTrf.m_pos, Mat3(finalDiffTrf.m_rot), Vec3(finalDiffTrf.m_scale));
		}
	}

	m_animationsStarted = std::move(m_pendingAnimationsStarted);
	m_animationsEnded = std::move(m_pendingAnimationsEnded);
}

Error AnimationComponent::serialize(SceneSerializer& serializer)
{
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

		if(serializer.isInReadMode()
		   && (track.m_crntAnimationIdx > track.m_lastAnimationIdx || track.m_lastAnimationIdx - track.m_crntAnimationIdx >= kMaxAnimationsPerTrack))
		{
			ANKI_SCENE_LOGE("Animation track has wrong indices: current {}, last {}", track.m_crntAnimationIdx, track.m_lastAnimationIdx);
			return Error::kUserData;
		}

		for(Animation& anim : track.m_anims)
		{
			ANKI_SERIALIZE(anim.m_resource, 1);
			ANKI_SERIALIZE(anim.m_blendInTime, 1);
			ANKI_SERIALIZE(anim.m_start, 1);
			ANKI_SERIALIZE(anim.m_animId, 1);
			ANKI_SERIALIZE(anim.m_channelIdx, 1);
			ANKI_SERIALIZE(anim.m_state.m_repeatTimes, 1);
			ANKI_SERIALIZE(anim.m_state.m_animationSpeedScale, 1);
			ANKI_SERIALIZE(anim.m_state.m_animationTime, 1);
			ANKI_SERIALIZE(anim.m_state.m_animationTimeBias, 1);

			if(serializer.isInReadMode() && anim.m_resource && anim.m_channelIdx >= anim.m_resource->getChannels().getSize())
			{
				ANKI_SCENE_LOGE("Animation channel index is out of bounds: {}", anim.m_channelIdx);
				return Error::kUserData;
			}
		}
	}

	ANKI_SERIALIZE(m_motionMode, 1);

	if(serializer.isInReadMode() && m_motionMode >= NodeMotionMode::kCount)
	{
		ANKI_SCENE_LOGE("Wrong node motion mode: {}", U32(m_motionMode));
		return Error::kUserData;
	}

	return Error::kNone;
}

} // end namespace anki
