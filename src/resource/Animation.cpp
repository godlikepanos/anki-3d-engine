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
		throw ANKI_EXCEPTION("Animation loading failed: " + filename) << e;
	}
}

//==============================================================================
void Animation::loadInternal(const XmlElement& el)
{
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

		// <positionKeys>
		XmlElement keysEl = chEl.getChildElement("positionsKeys");
		XmlElement keyEl = keysEl.getChildElement("key");
		do
		{
			Key<Vec3> key;

			// <time>
			key.time = keyEl.getFloat();

			// <value>
			key.value = keyEl.getVec3();

			// push_back
			ch.positions.push_back(key);

			// Move to next
			keyEl = keyEl.getNextSiblingElement("key");
		} while(keyEl);

		// <rotationKeys>
		keysEl = chEl.getChildElement("rotationKeys");
		keyEl = keysEl.getChildElement("key");
		do
		{
			Key<Quat> key;

			// <time>
			key.time = keyEl.getFloat();

			// <value>
			key.value = Quat(keyEl.getVec4());

			// push_back
			ch.rotations.push_back(key);

			// Move to next
			keyEl = keyEl.getNextSiblingElement("key");
		} while(keyEl);

		// <scalingKeys>
		keysEl = chEl.getChildElementOptional("scalingKeys");
		if(keysEl)
		{
			XmlElement keyEl = keysEl.getChildElement("key");
			do
			{
				Key<F32> key;

				// <time>
				key.time = keyEl.getFloat();

				// <value>
				key.value = keyEl.getFloat();

				// push_back
				ch.scales.push_back(key);

				// Move to next
				keyEl = keyEl.getNextSiblingElement("key");
			} while(keyEl);
		}

		// Move to next channel
		chEl = chEl.getNextSiblingElement("channel");
	} while(chEl);
}

} // end namespace anki
