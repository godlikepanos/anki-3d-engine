#include "anki/resource/Animation.h"
#include "anki/misc/Xml.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
void Animation::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename);
		loadInternal(doc.getChildElement("animation"));
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load animation") << e;
	}
}

//==============================================================================
void Animation::loadInternal(const XmlElement& el)
{
	// Count the number of identity keys. If all of the keys are identities
	// drop a vector
	U identPosCount = 0;
	U identRotCount = 0;
	U identScaleCount = 0;

	// <duration>
	duration = el.getChildElement("duration").getFloat();

	// <channels>
	XmlElement channelsEl = el.getChildElement("channels");
	XmlElement chEl = channelsEl.getChildElement("channel");

	do
	{
		channels.push_back(AnimationChannel());	
		AnimationChannel& ch = channels.back();

		// <name>
		ch.name = chEl.getChildElement("name").getText();

		XmlElement keysEl, keyEl;

		// <positionKeys>
		keysEl = chEl.getChildElementOptional("positionKeys");
		if(keysEl)
		{
			keyEl = keysEl.getChildElement("key");
			do
			{
				Key<Vec3> key;

				// <time>
				key.time = keyEl.getChildElement("time").getFloat();

				// <value>
				key.value = keyEl.getChildElement("value").getVec3();

				// push_back
				ch.positions.push_back(key);

				// Check ident
				if(key.value == Vec3(0.0))
				{
					++identPosCount;
				}

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// <rotationKeys>
		keysEl = chEl.getChildElement("rotationKeys");
		if(keysEl)
		{
			keyEl = keysEl.getChildElement("key");
			do
			{
				Key<Quat> key;

				// <time>
				key.time = keyEl.getChildElement("time").getFloat();

				// <value>
				key.value = Quat(keyEl.getChildElement("value").getVec4());

				// push_back
				ch.rotations.push_back(key);

				// Check ident
				if(key.value == Quat::getIdentity())
				{
					++identRotCount;
				}

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// <scalingKeys>
		keysEl = chEl.getChildElementOptional("scalingKeys");
		if(keysEl)
		{
			XmlElement keyEl = keysEl.getChildElement("key");
			do
			{
				Key<F32> key;

				// <time>
				key.time = keyEl.getChildElement("time").getFloat();

				// <value>
				key.value = keyEl.getChildElement("value").getFloat();

				// push_back
				ch.scales.push_back(key);

				// Check ident
				if(isZero(key.value - 1.0))
				{
					++identScaleCount;
				}

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// Remove identity vectors
		if(identPosCount == ch.positions.size())
		{
			ch.positions.clear();
		}
		if(identRotCount == ch.rotations.size())
		{
			ch.rotations.clear();
		}
		if(identScaleCount == ch.scales.size())
		{
			ch.scales.clear();
		}

		// Move to next channel
		chEl = chEl.getNextSiblingElement("channel");
	} while(chEl);
}

//==============================================================================
void Animation::interpolate(U channelIndex, F32 time, 
	Vec3& pos, Quat& rot, F32& scale) const
{
	ANKI_ASSERT(time >= startTime && time <= startTime + duration);
	ANKI_ASSERT(channelIndex < channels.size());
	
	const AnimationChannel& channel = channels[channelIndex];

	// Position
	if(channel.positions.size() > 1)
	{
		auto next = channel.positions.begin();

		do
		{
		} while((next->time < time) && (++next != channel.positions.end()));

		ANKI_ASSERT(next != channel.positions.end());
		auto prev = next - 1;
		/*auto prevprev = 
			(prev == channel.positions.begin()) ? prev : prev - 1;
		auto nextnext = 
			(next == channel.positions.end() - 1) ? next : next + 1;*/

		F32 u = (time - prev->time) / (next->time - prev->time);
		pos = linearInterpolate(prev->value, next->value, u);
	}

	// Rotation
	if(channel.rotations.size() > 1)
	{
		auto next = channel.rotations.begin();

		do
		{
		} while((next->time < time) && (++next != channel.rotations.end()));

		ANKI_ASSERT(next != channel.rotations.end());
		auto prev = next - 1;

		F32 u = (time - prev->time) / (next->time - prev->time);
		rot = prev->value.slerp(next->value, u);
	}
}

} // end namespace anki
