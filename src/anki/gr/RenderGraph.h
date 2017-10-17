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
using RenderTargetHandle = U32;

/// Buffer handle used in the RenderGraph.
using RenderPassBufferHandle = U32;

/// Describes the render target.
class RenderTargetDescription : public TextureInitInfo
{
	friend class RenderGraphDescription;

public:
	/// Create an internal hash.
	void bake()
	{
		m_hash = computeHash();
	}

private:
	U64 m_hash = 0;
};

/// Work callback for a RenderGraph pass.
using RenderPassWorkCallback = void (*)(
	void* userData, CommandBufferPtr cmdb, U32 secondLevelCmdbIdx, U32 secondLevelCmdbCount, const RenderGraph& rgraph);

/// RenderGraph pass dependency.
class RenderPassDependency
{
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	/// Dependency to an individual surface.
	RenderPassDependency(RenderTargetHandle handle, TextureUsageBit usage, const TextureSurfaceInfo& surface)
		: m_texture({handle, usage, surface, false})
		, m_isTexture(true)
	{
	}

	/// Dependency to the whole texture.
	RenderPassDependency(RenderTargetHandle handle, TextureUsageBit usage)
		: m_texture({handle, usage, TextureSurfaceInfo(0, 0, 0, 0), true})
		, m_isTexture(true)
	{
	}

	RenderPassDependency(RenderPassBufferHandle handle, BufferUsageBit usage)
		: m_buffer({handle, usage})
		, m_isTexture(false)
	{
	}

private:
	struct TextureInfo
	{
		RenderTargetHandle m_handle;
		TextureUsageBit m_usage;
		TextureSurfaceInfo m_surface;
		Bool8 m_wholeTex;
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
		ANKI_ASSERT(m_type == Type::GRAPHICS || secondLeveCmdbCount != 0);
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
			m_consumerRtMask.set(dep.m_texture.m_handle);
		}
		else if(dep.m_buffer.m_usage != BufferUsageBit::NONE)
		{
			m_consumerBufferMask.set(dep.m_buffer.m_handle);
		}
	}

	/// Add new producer dependency.
	void newProducer(const RenderPassDependency& dep)
	{
		m_producers.emplaceBack(m_alloc, dep);
		if(dep.m_isTexture)
		{
			m_producerRtMask.set(dep.m_texture.m_handle);
		}
		else
		{
			m_producerBufferMask.set(dep.m_buffer.m_handle);
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

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> m_consumerRtMask = {false};
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> m_producerRtMask = {false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS> m_consumerBufferMask = {false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS> m_producerBufferMask = {false};

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

/// Describes a framebuffer.
class GraphicsRenderPassFramebufferDescription
{
	friend class GraphicsRenderPassDescription;

public:
	void attachRenderTarget(U32 location,
		const TextureSurfaceInfo& surf,
		AttachmentLoadOperation loadOperation = AttachmentLoadOperation::DONT_CARE,
		AttachmentStoreOperation storeOperation = AttachmentStoreOperation::STORE,
		const ClearValue& clearValue = ClearValue())
	{
		FramebufferAttachmentInfo& att = m_fbInitInfo.m_colorAttachments[location];
		att.m_surface = surf;
		att.m_loadOperation = loadOperation;
		att.m_storeOperation = storeOperation;
		att.m_clearValue = clearValue;

		m_fbInitInfo.m_colorAttachmentCount = location + 1;
		m_defaultFb = false;
	}

	void attachDepthStencilRenderTarget(const TextureSurfaceInfo& surf,
		AttachmentLoadOperation loadOperation = AttachmentLoadOperation::CLEAR,
		AttachmentStoreOperation storeOperation = AttachmentStoreOperation::STORE,
		AttachmentLoadOperation stencilLoadOperation = AttachmentLoadOperation::CLEAR,
		AttachmentStoreOperation stencilStoreOperation = AttachmentStoreOperation::STORE,
		DepthStencilAspectBit aspect = DepthStencilAspectBit::DEPTH,
		F32 depthClear = 1.0f,
		I32 stencilClear = 0)
	{
		ANKI_ASSERT(!!(aspect & DepthStencilAspectBit::DEPTH_STENCIL));

		FramebufferAttachmentInfo& att = m_fbInitInfo.m_depthStencilAttachment;
		att.m_surface = surf;
		att.m_loadOperation = loadOperation;
		att.m_storeOperation = storeOperation;
		att.m_clearValue.m_depthStencil.m_depth = depthClear;
		att.m_clearValue.m_depthStencil.m_stencil = stencilClear;
		att.m_stencilLoadOperation = stencilLoadOperation;
		att.m_stencilStoreOperation = stencilStoreOperation;
		att.m_aspect = aspect;

		m_defaultFb = false;
	}

	void setDefaultFramebuffer()
	{
		m_defaultFb = true;
	}

	/// Calculate the hash for the framebuffer.
	void bake();

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
	}

	void setFramebufferInfo(const GraphicsRenderPassFramebufferDescription& fbInfo,
		const Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS>& colorRenderTargetHandles,
		RenderTargetHandle depthStencilRenderTargetHandle)
	{
		ANKI_ASSERT(fbInfo.m_hash != 0 && "Forgot call GraphicsRenderPassFramebufferInfo::bake");
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
	}

private:
	Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS + 1> m_rtHandles = {};
	FramebufferInitInfo m_fbInitInfo;
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
		return m_renderTargets.getSize() - 1;
	}

	/// Get or create a new render target.
	RenderTargetHandle newRenderTarget(CString name, const RenderTargetDescription& initInf)
	{
		ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDescription::bake");
		RT& rt = m_renderTargets.emplaceBack(m_alloc);
		rt.m_initInfo = initInf;
		rt.m_hash = initInf.m_hash;
		rt.m_usage = TextureUsageBit::NONE;
		rt.setName(name);
		return m_renderTargets.getSize() - 1;
	}

	/// Import a buffer.
	RenderPassBufferHandle importBuffer(CString name, BufferPtr buff, BufferUsageBit usage)
	{
		Buffer& b = m_buffers.emplaceBack(m_alloc);
		b.setName(name);
		b.m_usage = usage;
		b.m_importedBuff = buff;
		return m_buffers.getSize() - 1;
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
	void runSecondLevel();
	/// @}

	/// @name 3rd step methods
	/// @{

	/// Will call a number of RenderPassWorkCallback that populate 1st level command buffers.
	void run();
	/// @}

	/// @name 2nd and 3rd step methods
	/// @{
	TexturePtr getTexture(RenderTargetHandle handle) const;
	BufferPtr getBuffer(RenderPassBufferHandle handle) const;
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

	BakeContext* newContext(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initBatches();
	void setBatchBarriers(const RenderGraphDescription& descr, BakeContext& ctx) const;

	TexturePtr getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash);
	FramebufferPtr getOrCreateFramebuffer(
		const FramebufferInitInfo& fbInit, const RenderTargetHandle* rtHandles, U64 hash);

	static Bool passADependsOnB(
		BakeContext& ctx, const RenderPassDescriptionBase& a, const RenderPassDescriptionBase& b);
	static Bool passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx);

	/// Dump the dependency graph into a file.
	ANKI_USE_RESULT Error dumpDependencyDotFile(
		const RenderGraphDescription& descr, const BakeContext& ctx, CString path) const;
	static StringAuto textureUsageToStr(StackAllocator<U8>& alloc, TextureUsageBit usage);
	static StringAuto bufferUsageToStr(StackAllocator<U8>& alloc, BufferUsageBit usage);
};
/// @}

} // end namespace anki
