// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Enums.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// XXX
using RenderGraphHandle = U32;

/// XXX
using RenderGraphWorkCallback = void (*)(
	void* userData, CommandBufferPtr cmdb, U32 secondLevelCmdbIdx, U32 secondLevelCmdbCount);

/// XXX
class RenderGraphDependency
{
public:
	RenderGraphHandle m_handle; ///< Buffer or render target handle.
	union
	{
		TextureUsageBit m_textureUsage;
		BufferUsageBit m_bufferUsage;
	};
};

/// XXX
class RenderTargetInfo
{
public:
	RenderGraphHandle m_renderTarget;
	TextureSurfaceInfo m_surface;
	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;
	ClearValue m_clearValue;

	AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::STORE;

	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE; ///< Relevant only for depth stencil textures.
};

/// XXX
class RenderGraph final : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::RENDER_GRAPH;

	RenderGraph(GrManager* manager, U64 hash, GrObjectCache* cache);

	// Non-copyable
	RenderGraph(const RenderGraph&) = delete;

	~RenderGraph();

	// Non-copyable
	RenderGraph& operator=(const RenderGraph&) = delete;

	void init()
	{
		// Do nothing, implement the method for the interface
	}

	/// @name 1st step methods
	/// @{
	RenderGraphHandle importRenderTarget(CString name, TexturePtr tex);

	RenderGraphHandle newRenderTarget(const TextureInitInfo& texInf);

	RenderGraphHandle importBuffer(CString name, BufferPtr buff);

	RenderGraphHandle newBuffer(CString name, PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);

	RenderGraphHandle registerRenderPass(CString name,
		WeakArray<RenderTargetInfo> colorAttachments,
		RenderTargetInfo depthStencilAttachment,
		WeakArray<RenderGraphDependency> consumers,
		WeakArray<RenderGraphDependency> producers,
		RenderGraphWorkCallback callback,
		void* userData,
		U32 secondLevelCmdbsCount);

	/// For compute or other work (mipmap gen).
	RenderGraphHandle registerNonRenderPass(CString name,
		WeakArray<RenderGraphDependency> consumers,
		WeakArray<RenderGraphDependency> producers,
		RenderGraphWorkCallback callback,
		void* userData);
	/// @}

	/// @name 2nd step methods
	/// @{
	void bake();
	/// @}

	/// @name 3rd step methods
	/// @{

	/// Will call a number of RenderGraphWorkCallback that populate 2nd level command buffers.
	void runSecondLevel();
	/// @}

	/// @name 4th step methods
	/// @{

	/// Will call a number of RenderGraphWorkCallback that populate 1st level command buffers.
	void run();
	/// @}

	/// @name 3rd and 4th step methods
	/// @{
	TexturePtr getTexture(RenderGraphHandle handle);
	BufferPtr getBuffer(RenderGraphHandle handle);
	/// @}

	/// @name 5th step methods
	/// @{

	/// Reset the graph for a new frame. All previously created RenderGraphHandle are invalid after that call.
	void reset();
	/// @}

private:
	static constexpr U MAX_PASSES = 128;
	static constexpr U32 TEXTURE_TYPE = 0x10000000;
	static constexpr U32 BUFFER_TYPE = 0x20000000;
	static constexpr U32 RT_TYPE = 0x40000000;

	GrManager* m_gr;
	StackAllocator<U8> m_tmpAlloc;

	/// Render targets of the same type+size+format.
	class RenderTargetCacheEntry
	{
	public:
		DynamicArray<TexturePtr> m_textures;
		U32 m_texturesInUse = 0;
	};

	HashMap<TextureInitInfo, RenderTargetCacheEntry*> m_renderTargetCache; ///< Imported render targets.
	HashMap<FramebufferInitInfo, FramebufferPtr> m_framebufferCache;

	// Forward declarations
	class PassBatch;
	class RenderTarget;
	class Pass;
	class BakeContext;

	template<typename T>
	class Storage
	{
	public:
		T* m_arr = nullptr;
		U32 m_count = 0;
		U32 m_storage = 0;

		T& operator[](U i)
		{
			ANKI_ASSERT(i < m_count);
			return m_arr[i];
		}

		const T& operator[](U i) const
		{
			ANKI_ASSERT(i < m_count);
			return m_arr[i];
		}

		T* begin()
		{
			return &m_arr[0];
		}

		const T* begin() const
		{
			return &m_arr[0];
		}

		T* end()
		{
			return &m_arr[m_count];
		}

		const T* end() const
		{
			return &m_arr[m_count];
		}

		U32 getSize() const
		{
			return m_count;
		}

		void pushBack(StackAllocator<U8>& alloc, T&& x);

		void reset()
		{
			m_arr = nullptr;
			m_count = 0;
			m_storage = 0;
		}
	};

	/// @name Runtime stuff
	/// @{
	Storage<RenderTarget> m_renderTargets;
	Storage<Pass> m_passes;
	/// @}

	RenderGraphHandle pushRenderTarget(CString name, TexturePtr tex, Bool imported);

	Bool passHasUnmetDependencies(const BakeContext& ctx, const Pass& pass) const;

	static Bool passADependsOnB(const Pass& a, const Pass& b);

	/// Dump the dependency graph into a file.
	ANKI_USE_RESULT Error dumpDependencyDotFile(const BakeContext& ctx, CString path) const;
};
/// @}

} // end namespace anki
