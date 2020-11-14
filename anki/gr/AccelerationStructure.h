// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Buffer.h>
#include <anki/Math.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// @memberof AccelerationStructureInitInfo
class BottomLevelAccelerationStructureInitInfo
{
public:
	BufferPtr m_indexBuffer;
	PtrSize m_indexBufferOffset = 0;
	U32 m_indexCount = 0;
	IndexType m_indexType = IndexType::COUNT;

	BufferPtr m_positionBuffer;
	PtrSize m_positionBufferOffset = 0;
	U32 m_positionStride = 0;
	Format m_positionsFormat = Format::NONE;
	U32 m_positionCount = 0;

	Bool isValid() const
	{
		if(m_indexBuffer.get() == nullptr || m_indexCount == 0 || m_indexType == IndexType::COUNT
		   || m_positionBuffer.get() == nullptr || m_positionStride == 0 || m_positionsFormat == Format::NONE
		   || m_positionCount == 0)
		{
			return false;
		}

		const PtrSize posRange = m_positionBufferOffset + PtrSize(m_positionStride) * m_positionCount;
		const PtrSize formatSize = getFormatBytes(m_positionsFormat);
		if(m_positionStride < formatSize)
		{
			return false;
		}

		if(posRange > m_positionBuffer->getSize())
		{
			return false;
		}

		const PtrSize idxStride = (m_indexType == IndexType::U16) ? 2 : 4;
		if(m_indexBufferOffset + idxStride * m_indexCount > m_indexBuffer->getSize())
		{
			return false;
		}

		return true;
	}
};

/// @memberof AccelerationStructureInitInfo
class AccelerationStructureInstance
{
public:
	AccelerationStructurePtr m_bottomLevel;
	Mat3x4 m_transform = Mat3x4::getIdentity();
	U32 m_hitgroupSbtRecordIndex = 0; ///< Points to a hitgroup SBT record.
	U8 m_mask = 0xFF; ///< A mask that this instance belongs to. Will be tested against what's in traceRayEXT().
};

/// @memberof AccelerationStructureInitInfo
class TopLevelAccelerationStructureInitInfo
{
public:
	ConstWeakArray<AccelerationStructureInstance> m_instances;

	Bool isValid() const
	{
		return m_instances.getSize() > 0;
	}
};

/// Acceleration struture init info.
/// @memberof AccelerationStructure
class AccelerationStructureInitInfo : public GrBaseInitInfo
{
public:
	AccelerationStructureType m_type = AccelerationStructureType::COUNT;
	BottomLevelAccelerationStructureInitInfo m_bottomLevel;
	TopLevelAccelerationStructureInitInfo m_topLevel;

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

		return (m_type == AccelerationStructureType::BOTTOM_LEVEL) ? m_bottomLevel.isValid() : m_topLevel.isValid();
	}
};

/// Acceleration structure GPU object.
class AccelerationStructure : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::ACCELERATION_STRUCTURE;

	AccelerationStructureType getType() const
	{
		ANKI_ASSERT(m_type != AccelerationStructureType::COUNT);
		return m_type;
	}

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
	static ANKI_USE_RESULT AccelerationStructure* newInstance(GrManager* manager,
															  const AccelerationStructureInitInfo& init);
};
/// @}

} // end namespace anki
