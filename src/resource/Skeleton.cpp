// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Skeleton.h"
#include "anki/misc/Xml.h"
#include "anki/util/StringList.h"

namespace anki {

//==============================================================================
void Skeleton::load(const CString& filename, ResourceInitializer& init)
{
	XmlDocument doc;
	doc.loadFile(filename, init.m_tempAlloc);

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
	m_bones = std::move(ResourceVector<Bone>(init.m_alloc));
	m_bones.resize(bonesCount, Bone(init.m_alloc));

	// Load every bone
	boneEl = bonesEl.getChildElement("bone");
	bonesCount = 0;
	do
	{
		Bone& bone = m_bones[bonesCount++];

		// <name>
		XmlElement nameEl = boneEl.getChildElement("name");
		bone.m_name = nameEl.getText();

		// <transform>
		XmlElement trfEl = boneEl.getChildElement("transform");
		bone.m_transform = trfEl.getMat4();

		// Advance 
		boneEl = boneEl.getNextSiblingElement("bone");
	} while(boneEl);
}

} // end namespace anki
