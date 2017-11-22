// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Enums.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/util/HashMap.h>
#include <anki/util/BitSet.h>

namespace anki
{

// Forward
class RenderGraph;

/// @addtogroup graphics
/// @{

/// @name RenderGraph constants
/// @{
static constexpr U MAX_RENDER_GRAPH_PASSES = 128;
static constexpr U MAX_RENDER_GRAPH_RENDER_TARGETS = 128; ///< Max imported or not render targets in RenderGraph.
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

	void getRenderTargetState(
		RenderTargetHandle handle, TexturePtr& tex, TextureUsageBit& usage, DepthStencilAspectBit& aspect) const;
	void getRenderTargetState(RenderTargetHandle handle,
		const TextureSurfaceInfo& surf,
		TexturePtr& tex,
		TextureUsageBit& usage,
		DepthStencilAspectBit& aspect) const;
	void getRenderTargetState(RenderTargetHandle handle,
		const TextureVolumeInfo& vol,
		TexturePtr& tex,
		TextureUsageBit& usage,
		DepthStencilAspectBit& aspect) const;
	void getRenderTargetState(RenderTargetHandle handle,
		U32 level,
		TexturePtr& tex,
		TextureUsageBit& usage,
		DepthStencilAspectBit& aspect) const;

	/// Convenience method.
	void bindTexture(U32 set, U32 binding, RenderTargetHandle handle)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		DepthStencilAspectBit aspect;
		getRenderTargetState(handle, tex, usage, aspect);
		m_commandBuffer->bindTexture(set, binding, tex, usage, aspect);
	}

	/// Convenience method.
	void bindTextureAndSampler(U32 set, U32 binding, RenderTargetHandle handle, SamplerPtr sampler)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		DepthStencilAspectBit aspect;
		getRenderTargetState(handle, tex, usage, aspect);
		m_commandBuffer->bindTextureAndSampler(set, binding, tex, sampler, usage, aspect);
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
};

/// Work callback for a RenderGraph pass.
using RenderPassWorkCallback = void (*)(RenderPassWorkContext& ctx);

/// RenderGraph pass dependency.
class RenderPassDependency
{
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	/// Dependency to an individual surface.
	RenderPassDependency(RenderTargetHandle handle,
		TextureUsageBit usage,
		const TextureSurfaceInfo& surface,
		DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_texture({handle, usage, surface, false, aspect})
		, m_isTexture(true)
	{
		ANKI_ASSERT(handle.valid());
	}

	/// Dependency to the whole texture.
	RenderPassDependency(
		RenderTargetHandle handle, TextureUsageBit usage, DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_texture({handle, usage, TextureSurfaceInfo(0, 0, 0, 0), true, aspect})
		, m_isTexture(true)
	{
		ANKI_ASSERT(handle.valid());
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
		TextureSurfaceInfo m_surface;
		Bool8 m_wholeTex;
		DepthStencilAspectBit m_aspect;
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

public:
	virtual ~RenderPassDescriptionBase()
	{
		m_name.destroy(m_alloc); // To avoid the assertion
		m_consumers.destroy(m_alloc);
		m_producers.destroy(m_alloc);
	}

	void setWork(RenderPassWorkCallback callback, void* userData, U32 secondLeveCmdbCount)
	{
		ANKI_ASSERT(callback);
		ANKI_ASSERT(m_type == Type::GRAPHICS || secondLeveCmdbCount == 0);
		m_callback = callback;
		m_userData = userData;
		m_secondLevelCmdbsCount = secondLeveCmdbCount;
	}

	/// Add new consumer dependency.
	void newConsumer(const RenderPassDependency& dep)
	{
		m_consumers.emplaceBack(m_alloc, dep);

		if(dep.m_isTexture && dep.m_texture.m_usage != TextureUsageBit::NONE)
		{
			m_consumerRtMask.set(dep.m_texture.m_handle.m_idx);
		}
		else if(dep.m_buffer.m_usage != BufferUsageBit::NONE)
		{
			m_consumerBufferMask.set(dep.m_buffer.m_handle.m_idx);
		}
	}

	/// Add new producer dependency.
	void newProducer(const RenderPassDependency& dep)
	{
		m_producers.emplaceBack(m_alloc, dep);
		if(dep.m_isTexture)
		{
			m_producerRtMask.set(dep.m_texture.m_handle.m_idx);
		}
		else
		{
			m_producerBufferMask.set(dep.m_buffer.m_handle.m_idx);
		}
	}

protected:
	enum class Type : U8
	{
		GRAPHICS,
		NO_GRAPHICS
	};

	Type m_type;

	StackAllocator<U8> m_alloc;

	RenderPassWorkCallback m_callback = nullptr;
	void* m_userData = nullptr;
	U32 m_secondLevelCmdbsCount = 0;

	DynamicArray<RenderPassDependency> m_consumers;
	DynamicArray<RenderPassDependency> m_producers;

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> m_consumerRtMask = {false};
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> m_producerRtMask = {false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> m_consumerBufferMask = {false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> m_producerBufferMask = {false};

	String m_name;

	RenderPassDescriptionBase(Type t)
		: m_type(t)
	{
	}

	void setName(CString name)
	{
		m_name.create(m_alloc, (name.isEmpty()) ? "N/A" : name);
	}
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

public:
	Array<FramebufferDescriptionAttachment, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentCount = 0;
	FramebufferDescriptionAttachment m_depthStencilAttachment;

	void setDefaultFramebuffer()
	{
		m_defaultFb = true;
	}

	/// Calculate the hash for the framebuffer.
	void bake();

	Bool isBacked() const
	{
		return m_hash != 0;
	}

private:
	FramebufferInitInfo m_fbInitInfo;
	Bool8 m_defaultFb = false;

	U64 m_hash = 0;
};

/// A graphics render pass for RenderGraph.
class GraphicsRenderPassDescription : public RenderPassDescriptionBase
{
	friend class RenderGraphDescription;
	friend class RenderGraph;

public:
	GraphicsRenderPassDescription()
		: RenderPassDescriptionBase(Type::GRAPHICS)
	{
		memset(&m_rtHandles[0], 0xFF, sizeof(m_rtHandles));
	}

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
			if(fbInfo.m_defaultFb || i >= fbInfo.m_colorAttachmentCount)
			{
				ANKI_ASSERT(!colorRenderTargetHandles[i].isValid());
			}
			else
			{
				ANKI_ASSERT(colorRenderTargetHandles[i].isValid());
			}
		}

		if(fbInfo.m_defaultFb || !fbInfo.m_depthStencilAttachment.m_aspect)
		{
			ANKI_ASSERT(!depthStencilRenderTargetHandle.isValid());
		}
		else
		{
			ANKI_ASSERT(depthStencilRenderTargetHandle.isValid());
		}
#endif

		if(fbInfo.m_defaultFb)
		{
			m_fbInitInfo.m_colorAttachmentCount = 1;
		}
		else
		{
			m_fbInitInfo = fbInfo.m_fbInitInfo;
			memcpy(&m_rtHandles[0], &colorRenderTargetHandles[0], sizeof(colorRenderTargetHandles));
			m_rtHandles[MAX_COLOR_ATTACHMENTS] = depthStencilRenderTargetHandle;
		}
		m_fbInitInfo.setName(m_name.toCString());
		m_fbHash = fbInfo.m_hash;
		m_fbRenderArea = {{minx, miny, maxx, maxy}};
	}

private:
	Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS + 1> m_rtHandles;
	FramebufferInitInfo m_fbInitInfo;
	Array<U32, 4> m_fbRenderArea = {};
	U64 m_fbHash = 0;

	Bool hasFramebuffer() const
	{
		return m_fbHash != 0;
	}
};

/// A compute render pass for RenderGraph.
class ComputeRenderPassDescription : public RenderPassDescriptionBase
{
	friend class RenderGraphDescription;

public:
	ComputeRenderPassDescription()
		: RenderPassDescriptionBase(Type::NO_GRAPHICS)
	{
	}
};

/// Builds the description of the frame's render passes and their interactions.
class RenderGraphDescription
{
	friend class RenderGraph;

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
		GraphicsRenderPassDescription* pass = m_alloc.newInstance<GraphicsRenderPassDescription>();
		pass->m_alloc = m_alloc;
		pass->setName(name);
		m_passes.emplaceBack(m_alloc, pass);
		return *pass;
	}

	/// Create a new compute render pass.
	ComputeRenderPassDescription& newComputeRenderPass(CString name)
	{
		ComputeRenderPassDescription* pass = m_alloc.newInstance<ComputeRenderPassDescription>();
		pass->m_alloc = m_alloc;
		pass->setName(name);
		m_passes.emplaceBack(m_alloc, pass);
		return *pass;
	}

	/// Import an existing render target.
	RenderTargetHandle importRenderTarget(CString name, TexturePtr tex, TextureUsageBit usage)
	{
		RT& rt = m_renderTargets.emplaceBack(m_alloc);
		rt.m_importedTex = tex;
		rt.m_usage = usage;
		rt.setName(name);

		RenderTargetHandle out;
		out.m_idx = m_renderTargets.getSize() - 1;
		return out;
	}

	/// Get or create a new render target.
	RenderTargetHandle newRenderTarget(const RenderTargetDescription& initInf)
	{
		ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDescription::bake");
		RT& rt = m_renderTargets.emplaceBack(m_alloc);
		rt.m_initInfo = initInf;
		rt.m_hash = initInf.m_hash;
		rt.m_usage = TextureUsageBit::NONE;
		rt.setName(initInf.getName());

		RenderTargetHandle out;
		out.m_idx = m_renderTargets.getSize() - 1;
		return out;
	}

	/// Import a buffer.
	RenderPassBufferHandle importBuffer(CString name, BufferPtr buff, BufferUsageBit usage)
	{
		Buffer& b = m_buffers.emplaceBack(m_alloc);
		b.setName(name);
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
		TextureUsageBit m_usage;
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

	RenderGraph(GrManager* manager, U64 hash, GrObjectCache* cache);

	// Non-copyable
	RenderGraph(const RenderGraph&) = delete;

	~RenderGraph();

	// Non-copyable
	RenderGraph& operator=(const RenderGraph&) = delete;

	void init()
	{
		// Do nothing, implement the method to align with the general interface
	}

	/// @name 1st step methods
	/// @{
	void compileNewGraph(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	/// @}

	/// @name 2nd step methods
	/// @{

	/// Will call a number of RenderPassWorkCallback that populate 2nd level command buffers.
	void runSecondLevel(U32 threadIdx) const;
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

	BakeContext* newContext(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initBatches();
	void setBatchBarriers(const RenderGraphDescription& descr, BakeContext& ctx) const;

	TexturePtr getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash);
	FramebufferPtr getOrCreateFramebuffer(
		const FramebufferInitInfo& fbInit, const RenderTargetHandle* rtHandles, U64 hash);

	static Bool passADependsOnB(const RenderPassDescriptionBase& a, const RenderPassDescriptionBase& b);
	static Bool overlappingDependency(const RenderPassDependency& a, const RenderPassDependency& b);
	static Bool passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx);

	void getCrntUsageAndAspect(
		RenderTargetHandle handle, U32 passIdx, TextureUsageBit& usage, DepthStencilAspectBit& aspect) const;
	void getCrntUsageAndAspect(RenderTargetHandle handle,
		U32 passIdx,
		const TextureSurfaceInfo& surf,
		TextureUsageBit& usage,
		DepthStencilAspectBit& aspect) const;

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

inline void RenderPassWorkContext::getBufferState(RenderPassBufferHandle handle, BufferPtr& buff) const
{
	buff = m_rgraph->getBuffer(handle);
}

inline void RenderPassWorkContext::getRenderTargetState(
	RenderTargetHandle handle, TexturePtr& tex, TextureUsageBit& usage, DepthStencilAspectBit& aspect) const
{
	m_rgraph->getCrntUsageAndAspect(handle, m_passIdx, usage, aspect);
	tex = m_rgraph->getTexture(handle);
}

inline void RenderPassWorkContext::getRenderTargetState(RenderTargetHandle handle,
	const TextureSurfaceInfo& surf,
	TexturePtr& tex,
	TextureUsageBit& usage,
	DepthStencilAspectBit& aspect) const
{
	m_rgraph->getCrntUsageAndAspect(handle, m_passIdx, surf, usage, aspect);
	tex = m_rgraph->getTexture(handle);
}

inline void RenderPassWorkContext::getRenderTargetState(RenderTargetHandle handle,
	const TextureVolumeInfo& vol,
	TexturePtr& tex,
	TextureUsageBit& usage,
	DepthStencilAspectBit& aspect) const
{
	ANKI_ASSERT(!"TODO");
}

inline void RenderPassWorkContext::getRenderTargetState(
	RenderTargetHandle handle, U32 level, TexturePtr& tex, TextureUsageBit& usage, DepthStencilAspectBit& aspect) const
{
	ANKI_ASSERT(!"TODO");
}

} // end namespace anki
