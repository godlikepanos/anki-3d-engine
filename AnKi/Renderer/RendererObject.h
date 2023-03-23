// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Util/StdTypes.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Core/GpuMemoryPools.h>

namespace anki {

// Forward
class Renderer;
class ResourceManager;

/// @addtogroup renderer
/// @{

/// Renderer object.
class RendererObject
{
public:
	RendererObject(Renderer* r)
		: m_r(r)
	{
	}

	virtual ~RendererObject()
	{
	}

	HeapMemoryPool& getMemoryPool() const;

	virtual void getDebugRenderTarget([[maybe_unused]] CString rtName,
									  [[maybe_unused]] Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
									  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
	{
		ANKI_ASSERT(!"Object doesn't support that");
	}

protected:
	Renderer* m_r; ///< Know your father

	ANKI_PURE RendererExternalSubsystems& getExternalSubsystems();

	ANKI_PURE const RendererExternalSubsystems& getExternalSubsystems() const;

	void* allocateRebarStagingMemory(PtrSize size, RebarGpuMemoryToken& token);

	U32 computeNumberOfSecondLevelCommandBuffers(U32 drawcallCount) const;

	/// Used in fullscreen quad draws.
	static void drawQuad(CommandBufferPtr& cmdb)
	{
		cmdb->drawArrays(PrimitiveTopology::kTriangles, 3, 1);
	}

	/// Dispatch a compute job equivelent to drawQuad
	static void dispatchPPCompute(CommandBufferPtr& cmdb, U32 workgroupSizeX, U32 workgroupSizeY, U32 outImageWidth,
								  U32 outImageHeight)
	{
		const U32 sizeX = (outImageWidth + workgroupSizeX - 1) / workgroupSizeX;
		const U32 sizeY = (outImageHeight + workgroupSizeY - 1) / workgroupSizeY;
		cmdb->dispatchCompute(sizeX, sizeY, 1);
	}

	static void dispatchPPCompute(CommandBufferPtr& cmdb, U32 workgroupSizeX, U32 workgroupSizeY, U32 workgroupSizeZ,
								  U32 outImageWidth, U32 outImageHeight, U32 outImageDepth)
	{
		const U32 sizeX = (outImageWidth + workgroupSizeX - 1) / workgroupSizeX;
		const U32 sizeY = (outImageHeight + workgroupSizeY - 1) / workgroupSizeY;
		const U32 sizeZ = (outImageDepth + workgroupSizeZ - 1) / workgroupSizeZ;
		cmdb->dispatchCompute(sizeX, sizeY, sizeZ);
	}

	template<typename TPtr>
	TPtr allocateUniforms(PtrSize size, RebarGpuMemoryToken& token)
	{
		return static_cast<TPtr>(allocateRebarStagingMemory(size, token));
	}

	void bindUniforms(CommandBufferPtr& cmdb, U32 set, U32 binding, const RebarGpuMemoryToken& token) const;

	template<typename TPtr>
	TPtr allocateAndBindUniforms(PtrSize size, CommandBufferPtr& cmdb, U32 set, U32 binding)
	{
		RebarGpuMemoryToken token;
		TPtr ptr = allocateUniforms<TPtr>(size, token);
		bindUniforms(cmdb, set, binding, token);
		return ptr;
	}

	template<typename TPtr>
	TPtr allocateStorage(PtrSize size, RebarGpuMemoryToken& token)
	{
		return static_cast<TPtr>(allocateRebarStagingMemory(size, token));
	}

	void bindStorage(CommandBufferPtr& cmdb, U32 set, U32 binding, const RebarGpuMemoryToken& token) const;

	template<typename TPtr>
	TPtr allocateAndBindStorage(PtrSize size, CommandBufferPtr& cmdb, U32 set, U32 binding)
	{
		RebarGpuMemoryToken token;
		TPtr ptr = allocateStorage<TPtr>(size, token);
		bindStorage(cmdb, set, binding, token);
		return ptr;
	}

	void registerDebugRenderTarget(CString rtName);
};
/// @}

} // end namespace anki
