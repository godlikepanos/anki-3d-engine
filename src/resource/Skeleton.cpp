// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Skeleton.h"
#include "anki/misc/Xml.h"
#include "anki/util/StringList.h"

namespace anki {

//==============================================================================
Skeleton::Skeleton(ResourceAllocator<U8>& alloc)
:	m_bones(alloc)
{}

//==============================================================================
Skeleton::~Skeleton()
{}

//==============================================================================
void Skeleton::load(const CString& filename, ResourceInitializer& init)
{
	XmlDocument doc;
	doc.loadFile(filename, init.m_tempAlloc);

	XmlElement rootEl;
	doc.getChildElement("skeleton", rootEl);
	XmlElement bonesEl;
	rootEl.getChildElement("bones", bonesEl);

	// count the bones count
	U bonesCount = 0;

	XmlElement boneEl;
	bonesEl.getChildElement("bone", boneEl);

	do
	{
		++bonesCount;
		boneEl.getNextSiblingElement("bone", boneEl);
	} while(boneEl);

	// Alloc the vector
	m_bones.resize(bonesCount, Bone(init.m_alloc));

	// Load every bone
	bonesEl.getChildElement("bone", boneEl);
	bonesCount = 0;
	do
	{
		Bone& bone = m_bones[bonesCount++];

		// <name>
		XmlElement nameEl;
		boneEl.getChildElement("name", nameEl);
		CString tmp;
		nameEl.getText(tmp);
		bone.m_name = tmp;

		// <transform>
		XmlElement trfEl;
		boneEl.getChildElement("transform", trfEl);
		trfEl.getMat4(bone.m_transform);

		// Advance 
		boneEl.getNextSiblingElement("bone", boneEl);
	} while(boneEl);
}

} // end namespace anki
