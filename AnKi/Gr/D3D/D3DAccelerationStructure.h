// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// AccelerationStructure implementation.
class AccelerationStructureImpl final : public AccelerationStructure
{
public:
	AccelerationStructureImpl(CString name)
		: AccelerationStructure(name)
	{
	}

	~AccelerationStructureImpl();

	Error init(const AccelerationStructureInitInfo& inf);

	void fillBuildInfo(BufferView scratchBuff, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& buildDesc) const;

	D3D12_GLOBAL_BARRIER computeBarrierInfo(AccelerationStructureUsageBit before, AccelerationStructureUsageBit after) const;

	const Buffer& getAsBuffer() const
	{
		return *m_asBuffer;
	}

private:
	BufferInternalPtr m_asBuffer;

	class
	{
	public:
		BufferInternalPtr m_instancesBuff;
		PtrSize m_instancesBuffOffset = 0;
		U32 m_instanceCount = 0;
	} m_tlas;

	class
	{
	public:
		D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDesc;
	} m_blas;
};
/// @}

} // end namespace anki
