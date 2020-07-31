// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/SkeletonResource.h>
#include <anki/util/Xml.h>
#include <anki/util/StringList.h>

namespace anki
{

SkeletonResource::~SkeletonResource()
{
	for(Bone& b : m_bones)
	{
		b.destroy(getAllocator());
	}

	m_bones.destroy(getAllocator());
}

Error SkeletonResource::load(const ResourceFilename& filename, Bool async)
{
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("skeleton", rootEl));
	XmlElement bonesEl;
	ANKI_CHECK(rootEl.getChildElement("bones", bonesEl));

	// count the bones count
	XmlElement boneEl;
	U32 boneCount = 0;

	ANKI_CHECK(bonesEl.getChildElement("bone", boneEl));
	ANKI_CHECK(boneEl.getSiblingElementsCount(boneCount));
	++boneCount;

	m_bones.create(getAllocator(), boneCount);

	StringListAuto boneParents(getAllocator());

	// Load every bone
	boneCount = 0;
	do
	{
		Bone& bone = m_bones[boneCount];
		bone.m_idx = boneCount;

		// name
		CString name;
		ANKI_CHECK(boneEl.getAttributeText("name", name));
		bone.m_name.create(getAllocator(), name);

		// transform
		ANKI_CHECK(boneEl.getAttributeNumbers("transform", bone.m_transform));

		// boneTransform
		ANKI_CHECK(boneEl.getAttributeNumbers("boneTransform", bone.m_vertTrf));

		// parent
		CString parent;
		Bool hasParent;
		ANKI_CHECK(boneEl.getAttributeTextOptional("parent", parent, hasParent));
		if(hasParent)
		{
			boneParents.pushBack(parent);
		}
		else
		{
			boneParents.pushBack("");

			if(m_rootBoneIdx != MAX_U32)
			{
				ANKI_RESOURCE_LOGE("Skeleton cannot have more than one root nodes");
				return Error::USER_DATA;
			}

			m_rootBoneIdx = boneCount;
		}

		// Advance
		ANKI_CHECK(boneEl.getNextSiblingElement("bone", boneEl));
		++boneCount;
	} while(boneEl);

	// Resolve the parents
	auto it = boneParents.getBegin();
	for(U32 i = 0; i < m_bones.getSize(); ++i)
	{
		Bone& bone = m_bones[i];

		if(it->getLength() > 0)
		{
			for(U32 j = 0; j < m_bones.getSize(); ++j)
			{
				if(m_bones[j].m_name == *it)
				{
					bone.m_parent = &m_bones[j];
					break;
				}
			}

			if(bone.m_parent == nullptr)
			{
				ANKI_RESOURCE_LOGE("Bone \"%s\" is referencing an unknown parent \"%s\"", &bone.m_name[0],
								   &it->toCString()[0]);
				return Error::USER_DATA;
			}

			if(bone.m_parent->m_childrenCount >= MAX_CHILDREN_PER_BONE)
			{
				ANKI_RESOURCE_LOGE("Bone \"%s\" cannot have more that %u children", &bone.m_parent->m_name[0],
								   MAX_CHILDREN_PER_BONE);
				return Error::USER_DATA;
			}

			bone.m_parent->m_children[bone.m_parent->m_childrenCount++] = &bone;
		}

		++it;
	}

	return Error::NONE;
}

} // end namespace anki
