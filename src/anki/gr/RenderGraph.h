// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Enums.h>
#include <anki/gr/TextureView.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/util/HashMap.h>
#include <anki/util/BitSet.h>

namespace anki
{

// Forward
class RenderGraph;
class RenderGraphDescription;

/// @addtogroup graphics
/// @{

/// @name RenderGraph constants
/// @{
static constexpr U MAX_RENDER_GRAPH_PASSES = 128;
static constexpr U MAX_RENDER_GRAPH_RENDER_TARGETS = 64; ///< Max imported or not render targets in RenderGraph.
static constexpr U MAX_RENDER_GRAPH_BUFFERS = 64;
/// @}

/// Render target handle used in the RenderGraph.
class RenderTargetHandle
{
	friend class RenderPassDependency;
	friend class RenderGraphDescription;
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	bool operator==(const RenderTargetHandle& b) const
	{
		return m_idx == b.m_idx;
	}

	bool operator!=(const RenderTargetHandle& b) const
	{
		return m_idx != b.m_idx;
	}

	Bool isValid() const
	{
		return m_idx != MAX_U32;
	}

private:
	U32 m_idx = MAX_U32;

	Bool valid() const
	{
		return m_idx != MAX_U32;
	}
};

/// Buffer handle used in the RenderGraph.
class RenderPassBufferHandle
{
	friend class RenderPassDependency;
	friend class RenderGraphDescription;
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	operator BufferPtr() const;

	bool operator==(const RenderPassBufferHandle& b) const
	{
		return m_idx == b.m_idx;
	}

	bool operator!=(const RenderPassBufferHandle& b) const
	{
		return m_idx != b.m_idx;
	}

private:
	U32 m_idx = MAX_U32;

	Bool valid() const
	{
		return m_idx != MAX_U32;
	}
};

/// Describes the render target.
class RenderTargetDescription : public TextureInitInfo
{
	friend class RenderGraphDescription;

public:
	RenderTargetDescription()
	{
	}

	RenderTargetDescription(CString name)
		: TextureInitInfo(name)
	{
	}

	/// Create an internal hash.
	void bake()
	{
		ANKI_ASSERT(m_hash == 0);
		ANKI_ASSERT(m_usage == TextureUsageBit::NONE && "No need to supply the usage. RenderGraph will find out");
		m_hash = computeHash();
	}

private:
	U64 m_hash = 0;
};

/// The only parameter of RenderPassWorkCallback.
class RenderPassWorkContext
{
	friend class RenderGraph;

public:
	void* m_userData ANKI_DBG_NULLIFY; ///< The userData passed in RenderPassDescriptionBase::setWork
	CommandBufferPtr m_commandBuffer;
	U32 m_currentSecondLevelCommandBufferIndex ANKI_DBG_NULLIFY;
	U32 m_secondLevelCommandBufferCount ANKI_DBG_NULLIFY;

	void getBufferState(RenderPassBufferHandle handle, BufferPtr& buff) const;

	void getRenderTargetState(RenderTargetHandle handle,
		const TextureSubresourceInfo& subresource,
		TexturePtr& tex,
		TextureUsageBit& usage) const;

	/// Convenience method.
	void bindTextureAndSampler(U32 set,
		U32 binding,
		RenderTargetHandle handle,
		const TextureSubresourceInfo& subresource,
		const SamplerPtr& sampler)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		getRenderTargetState(handle, subresource, tex, usage);
		TextureViewInitInfo viewInit(tex, subresource, "TmpRenderGraph");
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindTextureAndSampler(set, binding, view, sampler, usage);
	}

	/// Convenience method to bind the whole texture as color.
	void bindColorTextureAndSampler(U32 set, U32 binding, RenderTargetHandle handle, const SamplerPtr& sampler)
	{
		TexturePtr tex = getTexture(handle);
		TextureViewInitInfo viewInit(tex); // Use the whole texture
		TextureUsageBit usage;
		getRenderTargetState(handle, viewInit, tex, usage);
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindTextureAndSampler(set, binding, view, sampler, usage);
	}

	/// Convenience method.
	void bindImage(U32 set, U32 binding, RenderTargetHandle handle, const TextureSubresourceInfo& subresource)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		getRenderTargetState(handle, subresource, tex, usage);
		TextureViewInitInfo viewInit(tex, subresource, "TmpRenderGraph");
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindImage(set, binding, view);
	}

	/// Convenience method.
	void bindStorageBuffer(U32 set, U32 binding, RenderPassBufferHandle handle)
	{
		BufferPtr buff;
		getBufferState(handle, buff);
		m_commandBuffer->bindStorageBuffer(set, binding, buff, 0, MAX_PTR_SIZE);
	}

	/// Convenience method.
	void bindUniformBuffer(U32 set, U32 binding, RenderPassBufferHandle handle)
	{
		BufferPtr buff;
		getBufferState(handle, buff);
		m_commandBuffer->bindUniformBuffer(set, binding, buff, 0, MAX_PTR_SIZE);
	}

private:
	const RenderGraph* m_rgraph ANKI_DBG_NULLIFY;
	U32 m_passIdx ANKI_DBG_NULLIFY;

	TexturePtr getTexture(RenderTargetHandle handle) const;
};

/// Work callback for a RenderGraph pass.
using RenderPassWorkCallback = void (*)(RenderPassWorkContext& ctx);

/// RenderGraph pass dependency.
class RenderPassDependency
{
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	/// Dependency to a texture subresource.
	RenderPassDependency(RenderTargetHandle handle, TextureUsageBit usage, const TextureSubresourceInfo& subresource)
		: m_texture({handle, usage, subresource})
		, m_isTexture(true)
	{
		ANKI_ASSERT(handle.valid());
	}

	/// Dependency to the whole texture.
	RenderPassDependency(
		RenderTargetHandle handle, TextureUsageBit usage, DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_texture({handle, usage, TextureSubresourceInfo()})
		, m_isTexture(true)
	{
		ANKI_ASSERT(handle.valid());
		m_texture.m_subresource.m_mipmapCount = MAX_U32; // Mark it as "whole texture"
		m_texture.m_subresource.m_depthStencilAspect = aspect;
	}

	RenderPassDependency(RenderPassBufferHandle handle, BufferUsageBit usage)
		: m_buffer({handle, usage})
		, m_isTexture(false)
	{
		ANKI_ASSERT(handle.valid());
	}

private:
	class TextureInfo
	{
	public:
		RenderTargetHandle m_handle;
		TextureUsageBit m_usage;
		TextureSubresourceInfo m_subresource;
	};

	struct BufferInfo
	{
		RenderPassBufferHandle m_handle;
		BufferUsageBit m_usage;
	};

	union
	{
		TextureInfo m_texture;
		BufferInfo m_buffer;
	};

	Bool8 m_isTexture;
};

/// The base of compute/transfer and graphics renderpasses for RenderGraph.
class RenderPassDescriptionBase
{
	friend class RenderGraph;
	friend class RenderGraphDescription;

public:
	virtual ~RenderPassDescriptionBase()
	{
		m_name.destroy(m_alloc); // To avoid the assertion
		m_rtDeps.destroy(m_alloc);
		m_buffDeps.destroy(m_alloc);
	}

	void setWork(RenderPassWorkCallback callback, void* userData, U32 secondLeveCmdbCount)
	{
		ANKI_ASSERT(callback);
		ANKI_ASSERT(m_type == Type::GRAPHICS || secondLeveCmdbCount == 0);
		m_callback = callback;
		m_userData = userData;
		m_secondLevelCmdbsCount = secondLeveCmdbCount;
	}

	/// Add a new consumer or producer dependency.
	void newDependency(const RenderPassDependency& dep);

protected:
	enum class Type : U8
	{
		GRAPHICS,
		NO_GRAPHICS
	};

	Type m_type;

	StackAllocator<U8> m_alloc;
	RenderGraphDescription* m_descr;

	RenderPassWorkCallback m_callback = nullptr;
	void* m_userData = nullptr;
	U32 m_secondLevelCmdbsCount = 0;

	DynamicArray<RenderPassDependency> m_rtDeps;
	DynamicArray<RenderPassDependency> m_buffDeps;

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> m_readRtMask = {false};
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> m_writeRtMask = {false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> m_readBuffMask = {false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> m_writeBuffMask = {false};
	Bool8 m_hasBufferDeps = false; ///< Opt.

	String m_name;

	RenderPassDescriptionBase(Type t, RenderGraphDescription* descr)
		: m_type(t)
		, m_descr(descr)
	{
		ANKI_ASSERT(descr);
	}

	void setName(CString name)
	{
		m_name.create(m_alloc, (name.isEmpty()) ? "N/A" : name);
	}

	void fixSubresource(RenderPassDependency& dep) const;

	void validateDep(const RenderPassDependency& dep);
};

/// Framebuffer attachment info.
class FramebufferDescriptionAttachment
{
public:
	TextureSurfaceInfo m_surface;
	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;
	ClearValue m_clearValue;

	AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::STORE;

	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE; ///< Relevant only for depth stencil textures.
};

/// Describes a framebuffer.
class FramebufferDescription
{
	friend class GraphicsRenderPassDescription;
	friend class RenderGraph;

public:
	Array<FramebufferDescriptionAttachment, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentCount = 0;
	FramebufferDescriptionAttachment m_depthStencilAttachment;

	/// Calculate the hash for the framebuffer.
	void bake();

	Bool isBacked() const
	{
		return m_hash != 0;
	}

private:
	U64 m_hash = 0;
};

/// A graphics render pass for RenderGraph.
class GraphicsRenderPassDescription : public RenderPassDescriptionBase
{
	friend class RenderGraphDescription;
	friend class RenderGraph;
	template<typename, typename>
	friend class GenericPoolAllocator;

public:
	void setFramebufferInfo(const FramebufferDescription& fbInfo,
		const Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS>& colorRenderTargetHandles,
		RenderTargetHandle depthStencilRenderTargetHandle,
		U32 minx = 0,
		U32 miny = 0,
		U32 maxx = MAX_U32,
		U32 maxy = MAX_U32)
	{
#if ANKI_EXTRA_CHECKS
		ANKI_ASSERT(fbInfo.isBacked() && "Forgot call GraphicsRenderPassFramebufferInfo::bake");
		for(U i = 0; i < colorRenderTargetHandles.getSize(); ++i)
		{
			if(i >= fbInfo.m_colorAttachmentCount)
			{
				ANKI_ASSERT(!colorRenderTargetHandles[i].isValid());
			}
			else
			{
				ANKI_ASSERT(colorRenderTargetHandles[i].isValid());
			}
		}

		if(!fbInfo.m_depthStencilAttachment.m_aspect)
		{
			ANKI_ASSERT(!depthStencilRenderTargetHandle.isValid());
		}
		else
		{
			ANKI_ASSERT(depthStencilRenderTargetHandle.isValid());
		}
#endif

		m_fbDescr = fbInfo;
		memcpy(&m_rtHandles[0], &colorRenderTargetHandles[0], sizeof(colorRenderTargetHandles));
		m_rtHandles[MAX_COLOR_ATTACHMENTS] = depthStencilRenderTargetHandle;
		m_fbRenderArea = {{minx, miny, maxx, maxy}};
	}

private:
	Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS + 1> m_rtHandles;
	FramebufferDescription m_fbDescr;
	Array<U32, 4> m_fbRenderArea = {};

	GraphicsRenderPassDescription(RenderGraphDescription* descr)
		: RenderPassDescriptionBase(Type::GRAPHICS, descr)
	{
		memset(&m_rtHandles[0], 0xFF, sizeof(m_rtHandles));
	}

	Bool hasFramebuffer() const
	{
		return m_fbDescr.m_hash != 0;
	}
};

/// A compute render pass for RenderGraph.
class ComputeRenderPassDescription : public RenderPassDescriptionBase
{
	friend class RenderGraphDescription;
	template<typename, typename>
	friend class GenericPoolAllocator;

private:
	ComputeRenderPassDescription(RenderGraphDescription* descr)
		: RenderPassDescriptionBase(Type::NO_GRAPHICS, descr)
	{
	}
};

/// Builds the description of the frame's render passes and their interactions.
class RenderGraphDescription
{
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	RenderGraphDescription(const StackAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	~RenderGraphDescription()
	{
		for(RenderPassDescriptionBase* pass : m_passes)
		{
			m_alloc.deleteInstance(pass);
		}
		m_passes.destroy(m_alloc);
		m_renderTargets.destroy(m_alloc);
		m_buffers.destroy(m_alloc);
	}

	/// Create a new graphics render pass.
	GraphicsRenderPassDescription& newGraphicsRenderPass(CString name)
	{
		GraphicsRenderPassDescription* pass = m_alloc.newInstance<GraphicsRenderPassDescription>(this);
		pass->m_alloc = m_alloc;
		pass->setName(name);
		m_passes.emplaceBack(m_alloc, pass);
		return *pass;
	}

	/// Create a new compute render pass.
	ComputeRenderPassDescription& newComputeRenderPass(CString name)
	{
		ComputeRenderPassDescription* pass = m_alloc.newInstance<ComputeRenderPassDescription>(this);
		pass->m_alloc = m_alloc;
		pass->setName(name);
		m_passes.emplaceBack(m_alloc, pass);
		return *pass;
	}

	/// Import an existing render target.
	RenderTargetHandle importRenderTarget(TexturePtr tex, TextureUsageBit usage)
	{
		RT& rt = *m_renderTargets.emplaceBack(m_alloc);
		rt.m_importedTex = tex;
		rt.m_importedLastKnownUsage = usage;
		rt.m_usageDerivedByDeps = TextureUsageBit::NONE;
		rt.setName(tex->getName());

		RenderTargetHandle out;
		out.m_idx = m_renderTargets.getSize() - 1;
		return out;
	}

	/// Get or create a new render target.
	RenderTargetHandle newRenderTarget(const RenderTargetDescription& initInf)
	{
		ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDescription::bake");
		ANKI_ASSERT(
			initInf.m_usage == TextureUsageBit::NONE && "Don't need to supply the usage. Render grap will find it");
		RT& rt = *m_renderTargets.emplaceBack(m_alloc);
		rt.m_initInfo = initInf;
		rt.m_hash = initInf.m_hash;
		rt.m_importedLastKnownUsage = TextureUsageBit::NONE;
		rt.m_usageDerivedByDeps = TextureUsageBit::NONE;
		rt.setName(initInf.getName());

		RenderTargetHandle out;
		out.m_idx = m_renderTargets.getSize() - 1;
		return out;
	}

	/// Import a buffer.
	RenderPassBufferHandle importBuffer(BufferPtr buff, BufferUsageBit usage)
	{
		Buffer& b = *m_buffers.emplaceBack(m_alloc);
		b.setName(buff->getName());
		b.m_usage = usage;
		b.m_importedBuff = buff;

		RenderPassBufferHandle out;
		out.m_idx = m_buffers.getSize() - 1;
		return out;
	}

private:
	class Resource
	{
	public:
		Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;

		void setName(CString name)
		{
			const U len = name.getLength();
			ANKI_ASSERT(len <= MAX_GR_OBJECT_NAME_LENGTH);
			strcpy(&m_name[0], (len) ? &name[0] : "unnamed");
		}
	};

	class RT : public Resource
	{
	public:
		TextureInitInfo m_initInfo;
		U64 m_hash = 0;
		TexturePtr m_importedTex;
		TextureUsageBit m_importedLastKnownUsage;
		TextureUsageBit m_usageDerivedByDeps; ///< XXX
	};

	class Buffer : public Resource
	{
	public:
		BufferUsageBit m_usage;
		BufferPtr m_importedBuff;
	};

	StackAllocator<U8> m_alloc;
	DynamicArray<RenderPassDescriptionBase*> m_passes;
	DynamicArray<RT> m_renderTargets;
	DynamicArray<Buffer> m_buffers;
};

/// Accepts a descriptor of the frame's render passes and sets the dependencies between them.
///
/// The idea for the RenderGraph is to automate:
/// - Synchronization (barriers, events etc) between passes.
/// - Command buffer creation for primary and secondary command buffers.
/// - Framebuffer creation.
/// - Render target creation (optional since textures can be imported as well).
///
/// It accepts a description of the frame's render passes (compute and graphics), compiles that description to calculate
/// dependencies and then populates command buffers with the help of multiple RenderPassWorkCallback.
class RenderGraph final : public GrObject
{
	ANKI_GR_OBJECT

	friend class RenderPassWorkContext;

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::RENDER_GRAPH;

	/// @name 1st step methods
	/// @{
	void compileNewGraph(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	/// @}

	/// @name 2nd step methods
	/// @{

	/// Will call a number of RenderPassWorkCallback that populate 2nd level command buffers.
	void runSecondLevel(U32 threadIdx);
	/// @}

	/// @name 3rd step methods
	/// @{

	/// Will call a number of RenderPassWorkCallback that populate 1st level command buffers.
	void run() const;
	/// @}

	/// @name 3rd step methods
	/// @{
	void flush();
	/// @}

	/// @name 4th step methods
	/// @{

	/// Reset the graph for a new frame. All previously created RenderGraphHandle are invalid after that call.
	void reset();
	/// @}

private:
	/// Render targets of the same type+size+format.
	class RenderTargetCacheEntry
	{
	public:
		DynamicArray<TexturePtr> m_textures;
		U32 m_texturesInUse = 0;
	};

	HashMap<U64, RenderTargetCacheEntry> m_renderTargetCache; ///< Non-imported render targets.

	HashMap<U64, FramebufferPtr> m_fbCache; ///< Framebuffer cache.

	// Forward declarations
	class BakeContext;
	class Pass;
	class Batch;
	class RT;
	class Buffer;
	class Barrier;

	BakeContext* m_ctx = nullptr;
	U64 m_version = 0;

	RenderGraph(GrManager* manager, CString name);

	~RenderGraph();

	static ANKI_USE_RESULT RenderGraph* newInstance(GrManager* manager);

	BakeContext* newContext(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initBatches();
	void setBatchBarriers(const RenderGraphDescription& descr);

	TexturePtr getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash);
	FramebufferPtr getOrCreateFramebuffer(const FramebufferDescription& fbDescr,
		const RenderTargetHandle* rtHandles,
		CString name,
		Bool& drawsToPresentableTex);

	ANKI_HOT static Bool passADependsOnB(const RenderPassDescriptionBase& a, const RenderPassDescriptionBase& b);

	static Bool overlappingTextureSubresource(const TextureSubresourceInfo& suba, const TextureSubresourceInfo& subb);

	static Bool passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx);

	void setTextureBarrier(Batch& batch, const RenderPassDependency& consumer);

	template<typename TFunc>
	static void iterateSurfsOrVolumes(const TexturePtr& tex, const TextureSubresourceInfo& subresource, TFunc func);

	void getCrntUsage(RenderTargetHandle handle,
		U32 passIdx,
		const TextureSubresourceInfo& subresource,
		TextureUsageBit& usage) const;

	/// @name Dump the dependency graph into a file.
	/// @{
	ANKI_USE_RESULT Error dumpDependencyDotFile(
		const RenderGraphDescription& descr, const BakeContext& ctx, CString path) const;
	static StringAuto textureUsageToStr(StackAllocator<U8>& alloc, TextureUsageBit usage);
	static StringAuto bufferUsageToStr(StackAllocator<U8>& alloc, BufferUsageBit usage);
	/// @}

	TexturePtr getTexture(RenderTargetHandle handle) const;
	BufferPtr getBuffer(RenderPassBufferHandle handle) const;
};
/// @}

} // end namespace anki

#include <anki/gr/RenderGraph.inl.h>
