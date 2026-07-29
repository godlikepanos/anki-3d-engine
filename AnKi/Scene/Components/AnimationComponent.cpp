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

AnimationComponent& AnimationComponent::setAnimationFilename(U32 track, CString fname)
{
	if(!ANKI_EXPECT(track < kMaxAnimationTracks))
	{
		return *this;
	}

	// Load
	AnimationResourcePtr newRsrc;
	if(ResourceManager::getSingleton().loadResource(fname, newRsrc))
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
		return *this;
	}

	// Init the track
	Track& t = m_tracks[track];
	t.m_anim = std::move(newRsrc);
	t.m_channel = 0; // Reset the channel

	m_validTracks.set(track);

	return *this;
}

CString AnimationComponent::getAnimationFilename(U32 track) const
{
	if(!ANKI_EXPECT(track < kMaxAnimationTracks))
	{
		return "*Error*";
	}

	if(!!m_tracks[track].m_anim)
	{
		return m_tracks[track].m_anim->getFilename();
	}
	else
	{
		ANKI_SCENE_LOGE("Track doesn't have an AnimationResource: %u", track);
		return "*Error*";
	}
}

AnimationComponent& AnimationComponent::setAnimationChannel(U32 track, CString channel)
{
	if(!ANKI_EXPECT(track < kMaxAnimationTracks && !!m_tracks[track].m_anim))
	{
		return *this;
	}

	Track& t = m_tracks[track];

	// Find channel
	U32 channelIdx = kMaxU32;
	if(channel.isEmpty())
	{
		channelIdx = 0;
	}
	else
	{
		for(U32 i = 0; i < t.m_anim->getChannels().getSize(); ++i)
		{
			if(t.m_anim->getChannels()[i].m_name == channel)
			{
				channelIdx = i;
				break;
			}
		}

		if(channelIdx == kMaxU32)
		{
			ANKI_SCENE_LOGE("Channel not found: %s", channel.cstr());
			return *this;
		}
	}

	t.m_channel = channelIdx;

	return *this;
}

CString AnimationComponent::getAnimationChannel(U32 track) const
{
	if(ANKI_EXPECT(track < kMaxAnimationTracks && !!m_tracks[track].m_anim))
	{
		return m_tracks[track].m_anim->getChannels()[m_tracks[track].m_channel].m_name.cstr();
	}
	else
	{
		return "*Error*";
	}
}

void AnimationComponent::resetTrack(U32 track)
{
	if(ANKI_EXPECT(track < kMaxAnimationTracks))
	{
		m_tracks[track] = {};
		m_validTracks.unset(track);
	}
}

void AnimationComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isValid()
#if !ANKI_WITH_EDITOR
	   || info.m_paused
#endif
	)
	{
		return;
	}

	updated = false;

	// Interpolate
	Vec3 blendedPos(0.0f);
	Vec3 addPos(0.0f);
	Quat blendedRot = Quat::getIdentity();
	Quat addRot = Quat::getIdentity();
	F32 blendedScale = 0.0f;
	F32 addScale = 1.0f;
	F32 weightSum = 0.0f;
	Bool first = true;
	for(Track& t : m_tracks)
	{
		AnimationState* pState = &t.m_state;
#if ANKI_WITH_EDITOR
		if(info.m_paused)
		{
			pState = &t.m_editorState;
		}
#endif
		AnimationState& state = *pState;

		if(!t.m_anim || state == AnimationState::kStopped)
		{
			continue;
		}

		const Second animTime = t.m_anim->getStartingTime() + t.m_relativeTimePassed;
		if(state == AnimationState::kPlaying)
		{
			t.m_relativeTimePassed += info.m_dt * Second(t.m_animationSpeedScale);

			if(t.m_wrapMode == AnimationWrapMode::kOnce && t.m_relativeTimePassed >= t.m_anim->getDuration())
			{
				state = AnimationState::kStopped;
				t.m_relativeTimePassed = 0.0;
			}
		}

		Vec3 pos;
		Quat rot;
		F32 scale = 1.0;
		t.m_anim->interpolate(t.m_channel, animTime, pos, rot, scale);

		if(t.m_blendMode == AnimationBlendMode::kBlend)
		{
			// This is an online (incremental) weighted mean

			const F32 w = t.m_blendWeight;
			if(first)
			{
				blendedPos = pos;
				blendedRot = rot;
				blendedScale = scale;
				weightSum = w;
				first = false;
			}
			else
			{
				weightSum += w;
				ANKI_ASSERT(weightSum > 0.0f);
				const F32 lerp = w / weightSum;
				blendedPos = blendedPos.lerp(pos, lerp);
				blendedRot = blendedRot.slerp(rot, lerp);
				blendedScale = linearInterpolate(blendedScale, scale, lerp);
			}
		}
		else
		{
			ANKI_ASSERT(t.m_blendMode == AnimationBlendMode::kAdditive);
			const F32 w = t.m_blendWeight;
			addPos += pos * w;
			addRot = Quat::getIdentity().slerp(rot, w) * addRot;
			addScale *= linearInterpolate(1.0f, scale, w);
		}

		updated = true;
	}

	// Update node
	if(updated)
	{
		const Bool haveBlend = weightSum > 0.0f;
		const Vec3 baseP = haveBlend ? blendedPos : Vec3(0.0f);
		const F32 baseS = haveBlend ? blendedScale : 1.0f;
		const Quat baseR = haveBlend ? blendedRot.normalize() : Quat::getIdentity();

		const Vec3 finalPos = baseP + addPos;
		const Quat finalRot = addRot * baseR;
		const F32 finalScale = baseS * addScale;

		Transform trf;
		trf.setOrigin(finalPos.xyz0);
		trf.setRotation(Mat3x4(Vec3(0.0f), finalRot));
		trf.setScale(Vec4(finalScale, finalScale, finalScale, 0.0f));
		info.m_node->setLocalTransform(trf);
	}
}

Error AnimationComponent::serialize(SceneSerializer& serializer)
{
	for(Track& t : m_tracks)
	{
		ANKI_SERIALIZE(t.m_anim, 1);
		ANKI_SERIALIZE(t.m_channel, 1);
		ANKI_SERIALIZE(t.m_relativeTimePassed, 1);
		ANKI_SERIALIZE(t.m_animationSpeedScale, 1);
		ANKI_SERIALIZE(t.m_blendWeight, 1);
		ANKI_SERIALIZE(t.m_state, 1);
		ANKI_SERIALIZE(t.m_wrapMode, 1);
		ANKI_SERIALIZE(t.m_blendMode, 1);

		if(serializer.isInReadMode())
		{
			if(t.m_anim && t.m_channel >= t.m_anim->getChannels().getSize())
			{
				ANKI_SCENE_LOGE("Animation channel index is out of bounds: %u", t.m_channel);
				return Error::kUserData;
			}

			m_validTracks.set(U32(&t - m_tracks.getBegin()), !!t.m_anim);
		}
	}

	return Error::kNone;
}

} // end namespace anki
