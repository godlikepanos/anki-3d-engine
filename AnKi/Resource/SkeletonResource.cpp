// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/SkeletonResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/StringList.h>

namespace anki {

Error SkeletonResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	ResourceXmlDocument doc;
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

	m_bones.resize(boneCount);

	ResourceStringList boneParents;

	// Load every bone
	boneCount = 0;
	do
	{
		Bone& bone = m_bones[boneCount];
		bone.m_idx = boneCount;

		// name
		CString name;
		ANKI_CHECK(boneEl.getAttributeText("name", name));
		bone.m_name = name;

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

		// Capsule
		F32 capsuleRadius = 0.0f;
		Bool hasCapsuleRadius;
		ANKI_CHECK(boneEl.getAttributeNumberOptional("capsuleRadius", capsuleRadius, hasCapsuleRadius));
		if(hasCapsuleRadius)
		{
			if(capsuleRadius < 0.0f)
			{
				ANKI_RESOURCE_LOGE("Wrong capsuleRadius: {}", capsuleRadius);
				return Error::kUserData;
			}

			bone.m_capsuleRadius = capsuleRadius;
		}

		F32 capsuleHalfHeight = 0.0f;
		Bool hasCapsuleHalfHeight;
		ANKI_CHECK(boneEl.getAttributeNumberOptional("capsuleHalfHeight", capsuleHalfHeight, hasCapsuleHalfHeight));
		if(hasCapsuleHalfHeight)
		{
			if(capsuleHalfHeight < 0.0f)
			{
				ANKI_RESOURCE_LOGE("Wrong capsuleHalfHeight: {}", capsuleHalfHeight);
				return Error::kUserData;
			}

			bone.m_capsuleHalfHeight = capsuleHalfHeight;
		}

		Vec3 capsulePosition;
		Bool hasCapsulePosition;
		ANKI_CHECK(boneEl.getAttributeNumbersOptional("capsulePosition", capsulePosition, hasCapsulePosition));
		if(hasCapsulePosition)
		{
			bone.m_capsulePosition = capsulePosition;
		}

		Vec3 capsuleRotation;
		Bool hasCapsuleRotation;
		ANKI_CHECK(boneEl.getAttributeNumbersOptional("capsuleRotation", capsuleRotation, hasCapsuleRotation));
		if(hasCapsuleRotation)
		{
			bone.m_capsuleRotation = Euler(toRad(capsuleRotation.x), toRad(capsuleRotation.y), toRad(capsuleRotation.z));
		}

		// Constraint
		Vec3 constraintRotation;
		Bool hasConstraintRotation;
		ANKI_CHECK(boneEl.getAttributeNumbersOptional("constraintRotation", constraintRotation, hasConstraintRotation));
		if(hasConstraintRotation)
		{
			bone.m_constraintRotation = Euler(toRad(constraintRotation.x), toRad(constraintRotation.y), toRad(constraintRotation.z));
		}

		Vec3 constraintParentRotation;
		Bool hasConstraintParentRotation;
		ANKI_CHECK(boneEl.getAttributeNumbersOptional("constraintParentRotation", constraintParentRotation, hasConstraintParentRotation));
		if(hasConstraintParentRotation)
		{
			bone.m_constraintParentRotation =
				Euler(toRad(constraintParentRotation.x), toRad(constraintParentRotation.y), toRad(constraintParentRotation.z));
		}

		auto readAngle = [&](CString attribName, F32 minDegrees, F32 maxDegrees, F32& outRadians) -> Error {
			F32 degrees;
			Bool hasAttrib;
			ANKI_CHECK(boneEl.getAttributeNumberOptional(attribName, degrees, hasAttrib));
			if(hasAttrib)
			{
				if(degrees < minDegrees || degrees > maxDegrees)
				{
					ANKI_RESOURCE_LOGE("Wrong {}: {}. Should be in [{}, {}]", attribName, degrees, minDegrees, maxDegrees);
					return Error::kUserData;
				}

				outRadians = toRad(degrees);
			}
			return Error::kNone;
		};

		ANKI_CHECK(readAngle("twistMinAngle", -180.0f, 180.0f, bone.m_twistMinAngle));
		ANKI_CHECK(readAngle("twistMaxAngle", -180.0f, 180.0f, bone.m_twistMaxAngle));
		ANKI_CHECK(readAngle("planeAngle", 0.0f, 180.0f, bone.m_planeAngle));
		ANKI_CHECK(readAngle("normalAngle", 0.0f, 180.0f, bone.m_normalAngle));

		if(bone.m_twistMinAngle > bone.m_twistMaxAngle)
		{
			ANKI_RESOURCE_LOGE("twistMinAngle can't be bigger than twistMaxAngle. Bone: {}", name);
			return Error::kUserData;
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
				ANKI_RESOURCE_LOGE("Bone \"{}\" is referencing an unknown parent \"{}\"", bone.m_name, *it);
				return Error::kUserData;
			}

			if(bone.m_parent->m_childrenCount >= kMaxChildrenPerBone)
			{
				ANKI_RESOURCE_LOGE("Bone \"{}\" cannot have more that {} children", bone.m_parent->m_name, kMaxChildrenPerBone);
				return Error::kUserData;
			}

			bone.m_parent->m_children[bone.m_parent->m_childrenCount++] = &bone;
		}

		++it;
	}

	return Error::kNone;
}

} // end namespace anki
