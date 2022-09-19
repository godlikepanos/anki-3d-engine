// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Math.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// @memberof AccelerationStructureInitInfo
class BottomLevelAccelerationStructureInitInfo
{
public:
	BufferPtr m_indexBuffer;
	PtrSize m_indexBufferOffset = 0;
	U32 m_indexCount = 0;
	IndexType m_indexType = IndexType::kCount;

	BufferPtr m_positionBuffer;
	PtrSize m_positionBufferOffset = 0;
	U32 m_positionStride = 0;
	Format m_positionsFormat = Format::kNone;
	U32 m_positionCount = 0;

	Bool isValid() const
	{
		if(m_indexBuffer.get() == nullptr || m_indexCount == 0 || m_indexType == IndexType::kCount
		   || m_positionBuffer.get() == nullptr || m_positionStride == 0 || m_positionsFormat == Format::kNone
		   || m_positionCount == 0)
		{
			return false;
		}

		const PtrSize posRange = m_positionBufferOffset + PtrSize(m_positionStride) * m_positionCount;
		const PtrSize formatSize = getFormatInfo(m_positionsFormat).m_texelSize;
		if(m_positionStride < formatSize)
		{
			return false;
		}

		if(posRange > m_positionBuffer->getSize())
		{
			return false;
		}

		const PtrSize idxStride = (m_indexType == IndexType::kU16) ? 2 : 4;
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
	AccelerationStructureType m_type = AccelerationStructureType::kCount;
	BottomLevelAccelerationStructureInitInfo m_bottomLevel;
	TopLevelAccelerationStructureInitInfo m_topLevel;

	AccelerationStructureInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	Bool isValid() const
	{
		if(m_type == AccelerationStructureType::kCount)
		{
			return false;
		}

		return (m_type == AccelerationStructureType::kBottomLevel) ? m_bottomLevel.isValid() : m_topLevel.isValid();
	}
};

/// Acceleration structure GPU object.
class AccelerationStructure : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kAccelerationStructure;

	AccelerationStructureType getType() const
	{
		ANKI_ASSERT(m_type != AccelerationStructureType::kCount);
		return m_type;
	}

protected:
	AccelerationStructureType m_type = AccelerationStructureType::kCount;

	/// Construct.
	AccelerationStructure(GrManager* manager, CString name)
		: GrObject(manager, kClassType, name)
	{
	}

	/// Destroy.
	~AccelerationStructure()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static AccelerationStructure* newInstance(GrManager* manager,
															const AccelerationStructureInitInfo& init);
};
/// @}

} // end namespace anki
