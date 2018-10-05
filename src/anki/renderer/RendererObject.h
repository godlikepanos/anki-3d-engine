// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/util/StdTypes.h>
#include <anki/Gr.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ShaderProgramResource.h>
#include <anki/core/StagingGpuMemoryManager.h>

namespace anki
{

// Forward
class Renderer;
class ResourceManager;
class ConfigSet;

/// @addtogroup renderer
/// @{

/// Renderer object.
class RendererObject
{
anki_internal:
	RendererObject(Renderer* r)
		: m_r(r)
	{
	}

	virtual ~RendererObject()
	{
	}

	HeapAllocator<U8> getAllocator() const;

protected:
	Renderer* m_r; ///< Know your father

	GrManager& getGrManager();
	const GrManager& getGrManager() const;

	ResourceManager& getResourceManager();

	void* allocateFrameStagingMemory(PtrSize size, StagingGpuMemoryType usage, StagingGpuMemoryToken& token);

	U32 computeNumberOfSecondLevelCommandBuffers(U32 drawcallCount) const;

	/// Used in fullscreen quad draws.
	static void drawQuad(CommandBufferPtr& cmdb)
	{
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3, 1);
	}

	/// Dispatch a compute job equivelent to drawQuad
	static void dispatchPPCompute(
		CommandBufferPtr& cmdb, U32 workgroupSizeX, U32 workgroupSizeY, U32 outImageWidth, U32 outImageHeight)
	{
		const U sizeX = (outImageWidth + workgroupSizeX - 1) / workgroupSizeX;
		const U sizeY = (outImageHeight + workgroupSizeY - 1) / workgroupSizeY;
		cmdb->dispatchCompute(sizeX, sizeY, 1);
	}

	static void dispatchPPCompute(CommandBufferPtr& cmdb,
		U32 workgroupSizeX,
		U32 workgroupSizeY,
		U32 workgroupSizeZ,
		U32 outImageWidth,
		U32 outImageHeight,
		U32 outImageDepth)
	{
		const U sizeX = (outImageWidth + workgroupSizeX - 1) / workgroupSizeX;
		const U sizeY = (outImageHeight + workgroupSizeY - 1) / workgroupSizeY;
		const U sizeZ = (outImageDepth + workgroupSizeZ - 1) / workgroupSizeZ;
		cmdb->dispatchCompute(sizeX, sizeY, sizeZ);
	}

	template<typename TPtr>
	TPtr allocateUniforms(PtrSize size, StagingGpuMemoryToken& token)
	{
		return static_cast<TPtr>(allocateFrameStagingMemory(size, StagingGpuMemoryType::UNIFORM, token));
	}

	void bindUniforms(CommandBufferPtr& cmdb, U set, U binding, const StagingGpuMemoryToken& token) const;

	template<typename TPtr>
	TPtr allocateAndBindUniforms(PtrSize size, CommandBufferPtr& cmdb, U set, U binding)
	{
		StagingGpuMemoryToken token;
		TPtr ptr = allocateUniforms<TPtr>(size, token);
		bindUniforms(cmdb, set, binding, token);
		return ptr;
	}

	template<typename TPtr>
	TPtr allocateStorage(PtrSize size, StagingGpuMemoryToken& token)
	{
		return static_cast<TPtr>(allocateFrameStagingMemory(size, StagingGpuMemoryType::STORAGE, token));
	}

	void bindStorage(CommandBufferPtr& cmdb, U set, U binding, const StagingGpuMemoryToken& token) const;

	template<typename TPtr>
	TPtr allocateAndBindStorage(PtrSize size, CommandBufferPtr& cmdb, U set, U binding)
	{
		StagingGpuMemoryToken token;
		TPtr ptr = allocateStorage<TPtr>(size, token);
		bindStorage(cmdb, set, binding, token);
		return ptr;
	}
};
/// @}

} // end namespace anki
