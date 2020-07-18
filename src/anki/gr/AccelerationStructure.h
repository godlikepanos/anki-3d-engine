// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Acceleration struture init info.
/// @memberof AccelerationStructure
class AccelerationStructureInitInfo : public GrBaseInitInfo
{
public:
	AccelerationStructureType m_type = AccelerationStructureType::COUNT;

	class
	{
	public:
		IndexType m_indexType = IndexType::COUNT;
		Format m_positionsFormat = Format::NONE;
		U32 m_indexCount = 0;
		U32 m_vertexCount = 0;
	} m_bottomLevel;

	class
	{
	public:
		U32 m_bottomLevelCount = 0;
	} m_topLevel;

	AccelerationStructureInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	Bool isValid() const
	{
		if(m_type == AccelerationStructureType::COUNT)
		{
			return false;
		}

		if(m_type == AccelerationStructureType::BOTTOM_LEVEL)
		{
			if(m_bottomLevel.m_indexType == IndexType::COUNT || m_bottomLevel.m_positionsFormat == Format::NONE
				|| m_bottomLevel.m_indexCount == 0 || m_bottomLevel.m_vertexCount == 0)
			{
				return false;
			}
		}
		else
		{
			if(m_topLevel.m_bottomLevelCount == 0)
			{
				return false;
			}
		}

		return true;
	}
};

/// Acceleration structure GPU object.
class AccelerationStructure : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::ACCELERATION_STRUCTURE;

protected:
	AccelerationStructureType m_type = AccelerationStructureType::COUNT;

	/// Construct.
	AccelerationStructure(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~AccelerationStructure()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT AccelerationStructure* newInstance(
		GrManager* manager, const AccelerationStructureInitInfo& init);
};
/// @}

} // end namespace anki
