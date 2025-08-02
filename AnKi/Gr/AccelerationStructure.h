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

	Bool isValid(Bool validateBuffers) const
	{
		Bool valid = true;

		valid = valid && (m_indexCount >= 3 && m_indexType != IndexType::kCount);

		if(validateBuffers)
		{
			valid = valid && (m_indexBuffer.isValid() && m_indexCount * getIndexSize(m_indexType) == m_indexBuffer.getRange());
		}

		const U32 vertSize = getFormatInfo(m_positionsFormat).m_texelSize;
		valid = valid && (m_positionStride >= vertSize && m_positionCount >= 3);

		if(validateBuffers)
		{
			valid = valid && (m_positionBuffer.isValid() && m_positionStride * m_positionCount == m_positionBuffer.getRange());
		}

		return valid;
	}
};

/// @memberof AccelerationStructureInitInfo
class TopLevelAccelerationStructureInitInfo
{
public:
	U32 m_instanceCount = 0;
	BufferView m_instancesBuffer; ///< Filled with AccelerationStructureInstance structs.

	Bool isValid(Bool validateBuffers) const
	{
		Bool valid = true;

		valid = valid && m_instanceCount > 0;

		if(validateBuffers)
		{
			valid = valid && (m_instancesBuffer.getRange() == sizeof(AccelerationStructureInstance) * m_instanceCount);
		}

		return valid;
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

	BufferView m_accelerationStructureBuffer; ///< Optionaly supply the buffer of the AS.

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

		return (m_type == AccelerationStructureType::kBottomLevel) ? m_bottomLevel.isValid(true) : m_topLevel.isValid(true);
	}

	Bool isValidForGettingMemoryRequirements() const
	{
		if(m_type == AccelerationStructureType::kCount)
		{
			return false;
		}

		return (m_type == AccelerationStructureType::kBottomLevel) ? m_bottomLevel.isValid(false) : m_topLevel.isValid(false);
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
	PtrSize m_scratchBufferSize = 0; ///< Contains more bytes than what the APIs report. This is done to avoid exposing the alignment.
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
