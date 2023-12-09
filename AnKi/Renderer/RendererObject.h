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
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>

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
	RendererObject() = default;

	virtual ~RendererObject() = default;

	virtual void getDebugRenderTarget([[maybe_unused]] CString rtName, [[maybe_unused]] Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
									  [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
	{
		ANKI_ASSERT(!"Object doesn't support that");
	}

protected:
	static ANKI_PURE Renderer& getRenderer();

	/// Used in fullscreen quad draws.
	static void drawQuad(CommandBuffer& cmdb)
	{
		cmdb.draw(PrimitiveTopology::kTriangles, 3, 1);
	}

	/// Dispatch a compute job equivelent to drawQuad
	static void dispatchPPCompute(CommandBuffer& cmdb, U32 workgroupSizeX, U32 workgroupSizeY, U32 outImageWidth, U32 outImageHeight)
	{
		const U32 sizeX = (outImageWidth + workgroupSizeX - 1) / workgroupSizeX;
		const U32 sizeY = (outImageHeight + workgroupSizeY - 1) / workgroupSizeY;
		cmdb.dispatchCompute(sizeX, sizeY, 1);
	}

	static void dispatchPPCompute(CommandBuffer& cmdb, U32 workgroupSizeX, U32 workgroupSizeY, U32 workgroupSizeZ, U32 outImageWidth,
								  U32 outImageHeight, U32 outImageDepth)
	{
		const U32 sizeX = (outImageWidth + workgroupSizeX - 1) / workgroupSizeX;
		const U32 sizeY = (outImageHeight + workgroupSizeY - 1) / workgroupSizeY;
		const U32 sizeZ = (outImageDepth + workgroupSizeZ - 1) / workgroupSizeZ;
		cmdb.dispatchCompute(sizeX, sizeY, sizeZ);
	}

	template<typename T>
	static T* allocateAndBindConstants(CommandBuffer& cmdb, U32 set, U32 binding)
	{
		T* ptr;
		const RebarAllocation alloc = RebarTransientMemoryPool::getSingleton().allocateFrame(1, ptr);
		ANKI_ASSERT(isAligned(alignof(T), ptrToNumber(ptr)));
		cmdb.bindConstantBuffer(set, binding, alloc);
		return ptr;
	}

	template<typename T>
	static T* allocateAndBindUav(CommandBuffer& cmdb, U32 set, U32 binding, U32 count = 1)
	{
		T* ptr;
		const RebarAllocation alloc = RebarTransientMemoryPool::getSingleton().allocateFrame(count, ptr);
		ANKI_ASSERT(isAligned(alignof(T), ptrToNumber(ptr)));
		cmdb.bindUavBuffer(set, binding, alloc);
		return ptr;
	}

	void registerDebugRenderTarget(CString rtName);

	static Error loadShaderProgram(CString filename, ShaderProgramResourcePtr& rsrc, ShaderProgramPtr& grProg)
	{
		ANKI_CHECK(loadShaderProgram(filename, {}, rsrc, grProg));
		return Error::kNone;
	}

	class SubMutation
	{
	public:
		CString m_mutatorName;
		MutatorValue m_value;
	};

	static Error loadShaderProgram(CString filename, ConstWeakArray<SubMutation> mutators, ShaderProgramResourcePtr& rsrc, ShaderProgramPtr& grProg,
								   ShaderTypeBit shaderTypes = ShaderTypeBit::kNone, CString technique = {});

	static void zeroBuffer(Buffer* buff);
};
/// @}

} // end namespace anki
