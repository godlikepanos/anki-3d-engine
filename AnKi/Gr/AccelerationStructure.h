// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	BufferView m_indexBuffer;
	U32 m_indexCount = 0;
	IndexType m_indexType = IndexType::kCount;

	BufferView m_positionBuffer;
	U32 m_positionStride = 0;
	Format m_positionsFormat = Format::kNone;
	U32 m_positionCount = 0;

	Bool isValid() const
	{
		Bool valid = true;

		valid = valid && (m_indexBuffer.isValid() && m_indexCount * getIndexSize(m_indexType) == m_indexBuffer.getRange());

		const U32 vertSize = getFormatInfo(m_positionsFormat).m_texelSize;
		valid = valid
				&& (m_positionBuffer.isValid() && m_positionStride >= vertSize && m_positionStride * m_positionCount == m_positionBuffer.getRange());

		return valid;
	}
};

/// @memberof AccelerationStructureInitInfo
class AccelerationStructureInstanceInfo
{
public:
	AccelerationStructure* m_bottomLevel = nullptr;
	Mat3x4 m_transform = Mat3x4::getIdentity();
	U32 m_hitgroupSbtRecordIndex = 0; ///< Points to a hitgroup SBT record.
	U8 m_mask = 0xFF; ///< A mask that this instance belongs to. Will be tested against what's in traceRayEXT().
};

/// @memberof AccelerationStructureInitInfo
class TopLevelAccelerationStructureInitInfo
{
public:
	class
	{
	public:
		ConstWeakArray<AccelerationStructureInstanceInfo> m_instances;
	} m_directArgs; ///< Pass some representation of the instances.

	class
	{
	public:
		U32 m_maxInstanceCount = 0;
		BufferView m_instancesBuffer; ///< Filled with AccelerationStructureInstance structs.
	} m_indirectArgs; ///< Pass the instances GPU buffer directly.

	Bool isValid() const
	{
		return m_directArgs.m_instances.getSize() > 0
			   || (m_indirectArgs.m_maxInstanceCount > 0 && m_indirectArgs.m_instancesBuffer.isValid()
				   && m_indirectArgs.m_instancesBuffer.getRange() == sizeof(AccelerationStructureInstance) * m_indirectArgs.m_maxInstanceCount);
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

	/// Get the size of the scratch buffer used in building this AS.
	PtrSize getBuildScratchBufferSize() const
	{
		ANKI_ASSERT(m_scratchBufferSize != 0);
		return m_scratchBufferSize;
	}

	U64 getGpuAddress() const;

protected:
	PtrSize m_scratchBufferSize = 0;
	AccelerationStructureType m_type = AccelerationStructureType::kCount;

	/// Construct.
	AccelerationStructure(CString name)
		: GrObject(kClassType, name)
	{
	}

	/// Destroy.
	~AccelerationStructure()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static AccelerationStructure* newInstance(const AccelerationStructureInitInfo& init);
};
/// @}

} // end namespace anki
