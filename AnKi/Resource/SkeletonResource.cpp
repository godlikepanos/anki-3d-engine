// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/SkeletonResource.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/StringList.h>

namespace anki {

SkeletonResource::~SkeletonResource()
{
	for(Bone& b : m_bones)
	{
		b.destroy(getMemoryPool());
	}

	m_bones.destroy(getMemoryPool());
}

Error SkeletonResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	XmlDocument doc(&getTempMemoryPool());
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

	m_bones.create(getMemoryPool(), boneCount);

	StringListRaii boneParents(&getMemoryPool());

	// Load every bone
	boneCount = 0;
	do
	{
		Bone& bone = m_bones[boneCount];
		bone.m_idx = boneCount;

		// name
		CString name;
		ANKI_CHECK(boneEl.getAttributeText("name", name));
		bone.m_name.create(getMemoryPool(), name);

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

			if(m_rootBoneIdx != kMaxU32)
			{
				ANKI_RESOURCE_LOGE("Skeleton cannot have more than one root nodes");
				return Error::kUserData;
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
				return Error::kUserData;
			}

			if(bone.m_parent->m_childrenCount >= kMaxChildrenPerBone)
			{
				ANKI_RESOURCE_LOGE("Bone \"%s\" cannot have more that %u children", &bone.m_parent->m_name[0],
								   kMaxChildrenPerBone);
				return Error::kUserData;
			}

			bone.m_parent->m_children[bone.m_parent->m_childrenCount++] = &bone;
		}

		++it;
	}

	return Error::kNone;
}

} // end namespace anki
