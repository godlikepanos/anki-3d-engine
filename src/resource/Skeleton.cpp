#include "anki/resource/Skeleton.h"
#include "anki/misc/Xml.h"
#include "anki/util/StringList.h"

namespace anki {

//==============================================================================
void Skeleton::load(const char* filename)
{
	XmlDocument doc;
	doc.loadFile(filename);

	XmlElement rootEl = doc.getChildElement("skeleton");
	XmlElement bonesEl = rootEl.getChildElement("bones");

	// count the bones count
	U bonesCount = 0;

	XmlElement boneEl = bonesEl.getChildElement("bone");

	do
	{
		++bonesCount;
		boneEl = boneEl.getNextSiblingElement("bone");
	} while(boneEl);

	// Alloc the vector
	bones.resize(bonesCount);

	// Load every bone
	boneEl = bonesEl.getChildElement("bone");
	bonesCount = 0;
	do
	{
		Bone& bone = bones[bonesCount++];

		// <name>
		XmlElement nameEl = boneEl.getChildElement("name");
		bone.name = nameEl.getText();

		// <transform>
		XmlElement trfEl = boneEl.getChildElement("transform");
		StringList list = StringList::splitString(trfEl.getText(), ' ');

		if(list.size() != 16)
		{
			throw ANKI_EXCEPTION("Expecting 16 floats for <transform>");
		}

		for(U i = 0; i < 16; i++)
		{
			bone.transform[i] = std::stof(list[i]);
		}

		// Advance 
		boneEl = boneEl.getNextSiblingElement("bone");
	} while(boneEl);
}

} // end namespace anki
