// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Animation.h"
#include "anki/misc/Xml.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
Animation::Animation(ResourceAllocator<U8>& alloc)
:	m_channels(alloc)
{}

//==============================================================================
Animation::~Animation()
{}

//==============================================================================
void Animation::load(const CString& filename, ResourceInitializer& init)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename, init.m_tempAlloc);
		XmlElement el;
		doc.getChildElement("animation", el);
		loadInternal(el, init);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load animation") << e;
	}
}

//==============================================================================
void Animation::loadInternal(
	const XmlElement& rootel, ResourceInitializer& init)
{
	XmlElement el;
	m_startTime = MAX_F32;
	F32 maxTime = MIN_F32;
	auto& alloc = init.m_alloc;

	// Count the number of identity keys. If all of the keys are identities
	// drop a vector
	U identPosCount = 0;
	U identRotCount = 0;
	U identScaleCount = 0;

	// <repeat>
	XmlElement repel;
	rootel.getChildElementOptional("repeat", repel);
	if(repel)
	{
		I64 tmp;
		repel.getI64(tmp);
		m_repeat = tmp;
	}
	else
	{
		m_repeat = false;
	}

	// <channels>
	XmlElement channelsEl;
	rootel.getChildElement("channels", channelsEl);
	XmlElement chEl;
	channelsEl.getChildElement("channel", chEl);

	// For all channels
	do
	{
		m_channels.emplace_back(alloc);
		AnimationChannel& ch = m_channels.back();

		// <name>
		chEl.getChildElement("name", el);
		ch.m_name = el.getText();

		XmlElement keysEl, keyEl;

		// <positionKeys>
		chEl.getChildElementOptional("positionKeys", keysEl);
		if(keysEl)
		{
			keysEl.getChildElement("key", keyEl);
			do
			{
				Key<Vec3> key;

				// <time>
				F64 tmp;
				keyEl.getChildElement("time", el);
				el.getF64(tmp);
				key.m_time = tmp;
				m_startTime = std::min(m_startTime, key.m_time);
				maxTime = std::max(maxTime, key.m_time);

				// <value>
				keyEl.getChildElement("value", el);
				el.getVec3(key.m_value);

				// push_back
				ch.m_positions.push_back(key);

				// Check ident
				if(key.m_value == Vec3(0.0))
				{
					++identPosCount;
				}

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// <rotationKeys>
		chEl.getChildElement("rotationKeys", keysEl);
		if(keysEl)
		{
			keysEl.getChildElement("key", keyEl);
			do
			{
				Key<Quat> key;

				// <time>
				F64 tmp;
				keyEl.getChildElement("time", el);
				el.getF64(tmp);
				key.m_time = tmp;
				m_startTime = std::min(m_startTime, key.m_time);
				maxTime = std::max(maxTime, key.m_time);

				// <value>
				Vec4 tmp2;
				keyEl.getChildElement("value", el);
				el.getVec4(tmp2);
				key.m_value = Quat(tmp);

				// push_back
				ch.m_rotations.push_back(key);

				// Check ident
				if(key.m_value == Quat::getIdentity())
				{
					++identRotCount;
				}

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// <scalingKeys>
		chEl.getChildElementOptional("scalingKeys", keysEl);
		if(keysEl)
		{
			XmlElement keyEl;
			keysEl.getChildElement("key", keyEl);
			do
			{
				Key<F32> key;

				// <time>
				F64 tmp;
				keyEl.getChildElement("time", el);
				el.getF64(tmp);
				key.m_time = tmp;
				m_startTime = std::min(m_startTime, key.m_time);
				maxTime = std::max(maxTime, key.m_time);

				// <value>
				keyEl.getChildElement("value", el);
				el.getF64(tmp);
				key.m_value = tmp;

				// push_back
				ch.m_scales.push_back(key);

				// Check ident
				if(isZero(key.m_value - 1.0))
				{
					++identScaleCount;
				}

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// Remove identity vectors
		if(identPosCount == ch.m_positions.size())
		{
			ch.m_positions.clear();
		}
		if(identRotCount == ch.m_rotations.size())
		{
			ch.m_rotations.clear();
		}
		if(identScaleCount == ch.m_scales.size())
		{
			ch.m_scales.clear();
		}

		// Move to next channel
		chEl = chEl.getNextSiblingElement("channel");
	} while(chEl);

	m_duration = maxTime - m_startTime;
}

//==============================================================================
void Animation::interpolate(U channelIndex, F32 time, 
	Vec3& pos, Quat& rot, F32& scale) const
{
	// Audjust time
	if(m_repeat && time > m_startTime + m_duration)
	{
		time = mod(time - m_startTime, m_duration) + m_startTime;
	}

	ANKI_ASSERT(time >= m_startTime && time <= m_startTime + m_duration);
	ANKI_ASSERT(channelIndex < m_channels.size());

	const AnimationChannel& channel = m_channels[channelIndex];

	// Position
	if(channel.m_positions.size() > 1)
	{
		auto next = channel.m_positions.begin();

		do
		{
		} while((next->m_time < time) && (++next != channel.m_positions.end()));

		ANKI_ASSERT(next != channel.m_positions.end());
		auto prev = next - 1;

		F32 u = (time - prev->m_time) / (next->m_time - prev->m_time);
		pos = linearInterpolate(prev->m_value, next->m_value, u);
	}

	// Rotation
	if(channel.m_rotations.size() > 1)
	{
		auto next = channel.m_rotations.begin();

		do
		{
		} while((next->m_time < time) && (++next != channel.m_rotations.end()));

		ANKI_ASSERT(next != channel.m_rotations.end());
		auto prev = next - 1;

		F32 u = (time - prev->m_time) / (next->m_time - prev->m_time);
		rot = prev->m_value.slerp(next->m_value, u);
	}
}

} // end namespace anki
