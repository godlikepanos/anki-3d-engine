// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Skeleton.h"
#include "anki/misc/Xml.h"
#include "anki/util/StringList.h"

namespace anki {

//==============================================================================
Skeleton::Skeleton(ResourceAllocator<U8>& alloc)
{}

//==============================================================================
Skeleton::~Skeleton()
{
	for(Bone& b : m_bones)
	{
		b._destroy(m_alloc);
	}

	m_bones.destroy(m_alloc);
}

//==============================================================================
Error Skeleton::load(const CString& filename, ResourceInitializer& init)
{
	m_alloc = init.m_alloc;

	XmlDocument doc;
	ANKI_CHECK(doc.loadFile(filename, init.m_tempAlloc));

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

	m_bones.create(m_alloc, bonesCount);

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
		bone.m_name.create(m_alloc, tmp);

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
