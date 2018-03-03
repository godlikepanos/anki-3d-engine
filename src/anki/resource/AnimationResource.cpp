// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/AnimationResource.h>
#include <anki/misc/Xml.h>

namespace anki
{

AnimationResource::AnimationResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

AnimationResource::~AnimationResource()
{
	for(AnimationChannel& ch : m_channels)
	{
		ch.destroy(getAllocator());
	}

	m_channels.destroy(getAllocator());
}

Error AnimationResource::load(const ResourceFilename& filename, Bool async)
{
	XmlElement el;
	I64 tmp;
	F64 ftmp;

	m_startTime = MAX_F64;
	F64 maxTime = MIN_F64;

	// Document
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));
	XmlElement rootel;
	ANKI_CHECK(doc.getChildElement("animation", rootel));

	// Count the number of identity keys. If all of the keys are identities drop a vector
	U identPosCount = 0;
	U identRotCount = 0;
	U identScaleCount = 0;

	// <repeat>
	XmlElement repel;
	ANKI_CHECK(rootel.getChildElementOptional("repeat", repel));
	if(repel)
	{
		ANKI_CHECK(repel.getNumber(tmp));
		m_repeat = tmp;
	}
	else
	{
		m_repeat = false;
	}

	// <channels>
	XmlElement channelsEl;
	ANKI_CHECK(rootel.getChildElement("channels", channelsEl));
	XmlElement chEl;
	ANKI_CHECK(channelsEl.getChildElement("channel", chEl));

	U32 channelCount = 0;
	ANKI_CHECK(chEl.getSiblingElementsCount(channelCount));
	if(channelCount == 0)
	{
		ANKI_RESOURCE_LOGE("Didn't found any channels");
		return Error::USER_DATA;
	}
	m_channels.create(getAllocator(), channelCount);

	// For all channels
	channelCount = 0;
	do
	{
		AnimationChannel& ch = m_channels[channelCount];

		// <name>
		ANKI_CHECK(chEl.getChildElement("name", el));
		CString strtmp;
		ANKI_CHECK(el.getText(strtmp));
		ch.m_name.create(getAllocator(), strtmp);

		XmlElement keysEl, keyEl;

		// <positionKeys>
		ANKI_CHECK(chEl.getChildElementOptional("positionKeys", keysEl));
		if(keysEl)
		{
			ANKI_CHECK(keysEl.getChildElement("key", keyEl));

			U32 count = 0;
			ANKI_CHECK(keyEl.getSiblingElementsCount(count));
			ch.m_positions.create(getAllocator(), count);

			count = 0;
			do
			{
				AnimationKeyframe<Vec3>& key = ch.m_positions[count++];

				// <time>
				ANKI_CHECK(keyEl.getChildElement("time", el));
				ANKI_CHECK(el.getNumber(ftmp));
				key.m_time = ftmp;
				m_startTime = std::min(m_startTime, key.m_time);
				maxTime = std::max(maxTime, key.m_time);

				// <value>
				ANKI_CHECK(keyEl.getChildElement("value", el));
				ANKI_CHECK(el.getVec3(key.m_value));

				// Check ident
				if(key.m_value == Vec3(0.0))
				{
					++identPosCount;
				}

				// Move to next
				ANKI_CHECK(keyEl.getNextSiblingElement("key", keyEl));
			} while(keyEl);
		}

		// <rotationKeys>
		ANKI_CHECK(chEl.getChildElement("rotationKeys", keysEl));
		if(keysEl)
		{
			ANKI_CHECK(keysEl.getChildElement("key", keyEl));

			U32 count = 0;
			ANKI_CHECK(keysEl.getSiblingElementsCount(count));
			ch.m_rotations.create(getAllocator(), count);

			count = 0;
			do
			{
				AnimationKeyframe<Quat>& key = ch.m_rotations[count++];

				// <time>
				ANKI_CHECK(keyEl.getChildElement("time", el));
				ANKI_CHECK(el.getNumber(ftmp));
				key.m_time = ftmp;
				m_startTime = std::min(m_startTime, key.m_time);
				maxTime = std::max(maxTime, key.m_time);

				// <value>
				Vec4 tmp2;
				ANKI_CHECK(keyEl.getChildElement("value", el));
				ANKI_CHECK(el.getVec4(tmp2));
				key.m_value = Quat(tmp2);

				// Check ident
				if(key.m_value == Quat::getIdentity())
				{
					++identRotCount;
				}

				// Move to next
				ANKI_CHECK(keyEl.getNextSiblingElement("key", keyEl));
			} while(keyEl);
		}

		// <scalingKeys>
		ANKI_CHECK(chEl.getChildElementOptional("scalingKeys", keysEl));
		if(keysEl)
		{
			ANKI_CHECK(keysEl.getChildElement("key", keyEl));

			U32 count = 0;
			ANKI_CHECK(keyEl.getSiblingElementsCount(count));
			ch.m_scales.create(getAllocator(), count);

			count = 0;
			do
			{
				AnimationKeyframe<F32>& key = ch.m_scales[count++];

				// <time>
				ANKI_CHECK(keyEl.getChildElement("time", el));
				ANKI_CHECK(el.getNumber(ftmp));
				key.m_time = ftmp;
				m_startTime = std::min(m_startTime, key.m_time);
				maxTime = std::max(maxTime, key.m_time);

				// <value>
				ANKI_CHECK(keyEl.getChildElement("value", el));
				ANKI_CHECK(el.getNumber(ftmp));
				key.m_value = ftmp;

				// Check ident
				if(isZero(key.m_value - 1.0))
				{
					++identScaleCount;
				}

				// Move to next
				ANKI_CHECK(keyEl.getNextSiblingElement("key", keyEl));
			} while(keyEl);
		}

		// Remove identity vectors
		if(identPosCount == ch.m_positions.getSize())
		{
			ch.m_positions.destroy(getAllocator());
		}

		if(identRotCount == ch.m_rotations.getSize())
		{
			ch.m_rotations.destroy(getAllocator());
		}

		if(identScaleCount == ch.m_scales.getSize())
		{
			ch.m_scales.destroy(getAllocator());
		}

		// Move to next channel
		++channelCount;
		ANKI_CHECK(chEl.getNextSiblingElement("channel", chEl));
	} while(chEl);

	m_duration = maxTime - m_startTime;

	return Error::NONE;
}

void AnimationResource::interpolate(U channelIndex, F64 time, Vec3& pos, Quat& rot, F32& scale) const
{
	// Audjust time
	if(m_repeat && time > m_startTime + m_duration)
	{
		time = mod(time - m_startTime, m_duration) + m_startTime;
	}

	ANKI_ASSERT(time >= m_startTime && time <= m_startTime + m_duration);
	ANKI_ASSERT(channelIndex < m_channels.getSize());

	const AnimationChannel& channel = m_channels[channelIndex];

	// Position
	if(channel.m_positions.getSize() > 1)
	{
		auto next = channel.m_positions.begin();

		do
		{
		} while((next->m_time < time) && (++next != channel.m_positions.end()));

		ANKI_ASSERT(next != channel.m_positions.end());
		auto prev = next - 1;

		F64 u = (time - prev->m_time) / (next->m_time - prev->m_time);
		pos = linearInterpolate(prev->m_value, next->m_value, u);
	}

	// Rotation
	if(channel.m_rotations.getSize() > 1)
	{
		auto next = channel.m_rotations.begin();

		do
		{
		} while((next->m_time < time) && (++next != channel.m_rotations.end()));

		ANKI_ASSERT(next != channel.m_rotations.end());
		auto prev = next - 1;

		F64 u = (time - prev->m_time) / (next->m_time - prev->m_time);
		rot = prev->m_value.slerp(next->m_value, u);
	}
}

} // end namespace anki
