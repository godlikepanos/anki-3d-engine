// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/StdTypes.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>

namespace anki {

// Forward
class ResourceManager;

// Renderer object.
class RendererObject
{
public:
	RendererObject() = default;

	virtual ~RendererObject() = default;

	virtual void getDebugRenderTarget([[maybe_unused]] CString rtName,
									  [[maybe_unused]] Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
									  [[maybe_unused]] DebugRenderTargetDrawStyle& drawStyle) const
	{
		ANKI_ASSERT(!"Object doesn't support that");
	}

protected:
	static ANKI_PURE Renderer& getRenderer();

	// Used in fullscreen quad draws.
	static void drawQuad(CommandBuffer& cmdb)
	{
		cmdb.draw(PrimitiveTopology::kTriangles, 3, 1);
	}

	// Dispatch a compute job equivelent to drawQuad
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
	static T* allocateAndBindConstants(CommandBuffer& cmdb, U32 reg, U32 space)
	{
		T* ptr;
		const BufferView alloc = RebarTransientMemoryPool::getSingleton().allocateConstantBuffer<T>(ptr);
		ANKI_ASSERT(isAligned(alignof(T), ptrToNumber(ptr)));
		cmdb.bindConstantBuffer(reg, space, alloc);
		return ptr;
	}

	template<typename T>
	static WeakArray<T> allocateAndBindSrvStructuredBuffer(CommandBuffer& cmdb, U32 reg, U32 space, U32 count = 1)
	{
		WeakArray<T> out;
		const BufferView alloc = RebarTransientMemoryPool::getSingleton().allocateStructuredBuffer<T>(count, out);
		ANKI_ASSERT(isAligned(alignof(T), ptrToNumber(out.getBegin())));
		cmdb.bindSrv(reg, space, alloc);
		return out;
	}

	template<typename T>
	static WeakArray<T> allocateAndBindUavStructuredBuffer(CommandBuffer& cmdb, U32 reg, U32 space, U32 count = 1)
	{
		WeakArray<T> out;
		const BufferView alloc = RebarTransientMemoryPool::getSingleton().allocateStructuredBuffer<T>(count, out);
		ANKI_ASSERT(isAligned(alignof(T), ptrToNumber(out.getBegin())));
		cmdb.bindUav(reg, space, alloc);
		return out;
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

	static Error loadShaderProgram(CString filename, std::initializer_list<SubMutation> mutators, ShaderProgramResourcePtr& rsrc,
								   ShaderProgramPtr& grProg, CString technique = {}, ShaderTypeBit shaderTypes = ShaderTypeBit::kNone)
	{
		return loadShaderProgram(filename, ConstWeakArray<SubMutation>(mutators.begin(), U32(mutators.size())), rsrc, grProg, technique, shaderTypes);
	}

	static Error loadShaderProgram(CString filename, ConstWeakArray<SubMutation> mutators, ShaderProgramResourcePtr& rsrc, ShaderProgramPtr& grProg,
								   CString technique = {}, ShaderTypeBit shaderTypes = ShaderTypeBit::kNone);

	static void zeroBuffer(BufferView buff);

	static void fillBuffer(ConstWeakArray<U8> data, BufferView buff);

	// Temp pass name.
	static CString generateTempPassName(const Char* fmt, ...);

	// Fill some buffers with some value. It's a **COMPUTE** dispatch.
	static void fillBuffers(CommandBuffer& cmdb, ConstWeakArray<BufferView> buffers, U32 value);

	// See fillBuffers
	static void fillBuffer(CommandBuffer& cmdb, BufferView buffer, U32 value)
	{
		fillBuffers(cmdb, ConstWeakArray<BufferView>(&buffer, 1), value);
	}

	static DummyGpuResources& getDummyGpuResources()
	{
		return Renderer::getSingleton().m_dummyResources;
	}

#define ANKI_RENDERER_OBJECT_DEF(type, name, initCondition) \
	ANKI_FORCE_INLINE type& get##type() \
	{ \
		return getRenderer().get##type(); \
	} \
	ANKI_FORCE_INLINE Bool is##type##Enabled() const \
	{ \
		return getRenderer().is##type##Enabled(); \
	}
#include <AnKi/Renderer/RendererObject.def.h>

	ANKI_PURE RenderingContext& getRenderingContext() const;
};

// Contains common functionality of all passes that use RtMaterialFetch.
class RtMaterialFetchRendererObject : protected RendererObject
{
protected:
	Error init();

	// Build a pass that populates the shader binding table.
	void buildShaderBindingTablePass(CString passName, BufferView shaderGroupHandlesBuff, U32 raygenGroupIdx, U32 missGroupIdx, U32 sbtRecordSize,
									 RenderGraphBuilder& rgraph, BufferHandle& sbtHandle, BufferView& sbtBuffer);

	void patchShaderBindingTablePass(CString passName, BufferView shaderGroupHandlesBuff, U32 raygenGroupIdx, U32 missGroupIdx, U32 sbtRecordSize,
									 RenderGraphBuilder& rgraph, BufferHandle sbtHandle, BufferView sbtBuffer);

	// Sets the the resources of space 2 in RtMaterialFetch.hlsl as dependencies on the given pass.
	void setRgenSpace2Dependencies(RenderPassBase& pass, Bool isComputeDispatch = false);

	// Bind the the resources of space 2 in RtMaterialFetch.hlsl.
	void bindRgenSpace2Resources(RenderPassWorkContext& rgraphCtx);

private:
	ShaderProgramResourcePtr m_sbtBuildProg;
	ShaderProgramPtr m_sbtBuildGrProg;
	ShaderProgramPtr m_sbtPatchGrProg;
};

} // end namespace anki
