// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Skeleton.h>
#include <anki/misc/Xml.h>
#include <anki/util/StringList.h>

namespace anki
{

Skeleton::~Skeleton()
{
	for(Bone& b : m_bones)
	{
		b._destroy(getAllocator());
	}

	m_bones.destroy(getAllocator());
}

Error Skeleton::load(const ResourceFilename& filename)
{
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("skeleton", rootEl));
	XmlElement bonesEl;
	ANKI_CHECK(rootEl.getChildElement("bones", bonesEl));

	// count the bones count
	XmlElement boneEl;
	U32 bonesCount = 0;

	ANKI_CHECK(bonesEl.getChildElement("bone", boneEl));
	ANKI_CHECK(boneEl.getSiblingElementsCount(bonesCount));
	++bonesCount;

	m_bones.create(getAllocator(), bonesCount);

	// Load every bone
	bonesCount = 0;
	do
	{
		Bone& bone = m_bones[bonesCount++];

		// <name>
		XmlElement nameEl;
		ANKI_CHECK(boneEl.getChildElement("name", nameEl));
		CString tmp;
		ANKI_CHECK(nameEl.getText(tmp));
		bone.m_name.create(getAllocator(), tmp);

		// <transform>
		XmlElement trfEl;
		ANKI_CHECK(boneEl.getChildElement("transform", trfEl));
		ANKI_CHECK(trfEl.getMat4(bone.m_transform));

		// Advance
		ANKI_CHECK(boneEl.getNextSiblingElement("bone", boneEl));
	} while(boneEl);

	return ErrorCode::NONE;
}

} // end namespace anki
