// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Enums.h>
#include <anki/gr/Texture.h>
#include <anki/util/HashMap.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// @name RenderGraph constants
/// @{
static constexpr U MAX_RENDER_GRAPH_PASSES = 128;
static constexpr U MAX_RENDER_GRAPH_RENDER_TARGETS = 128;
static constexpr U MAX_RENDER_GRAPH_BUFFERS = 64;
/// @}

/// XXX
using RenderTargetHandle = U32;

/// XXX
using RenderPassBufferHandle = U32;

/// XXX
using RenderPassWorkCallback = void (*)(
	void* userData, CommandBufferPtr cmdb, U32 secondLevelCmdbIdx, U32 secondLevelCmdbCount);

/// XXX
class RenderPassDependency
{
	friend class RenderGraph;
	friend class RenderPassBase;

public:
	struct TextureInfo
	{
		RenderTargetHandle m_handle;
		TextureUsageBit m_usage;
		TextureSurfaceInfo m_surface;
	};

	struct BufferInfo
	{
		RenderPassBufferHandle m_handle;
		BufferUsageBit m_usage;
		PtrSize m_offset;
		PtrSize m_range;
	};

	union
	{
		TextureInfo m_texture;
		BufferInfo m_buffer;
	};

	RenderPassDependency(RenderTargetHandle handle,
		TextureUsageBit usage,
		const TextureSurfaceInfo& surface = TextureSurfaceInfo(0, 0, 0, 0))
		: m_texture({handle, usage, surface})
		, m_isTexture(true)
	{
	}

	RenderPassDependency(
		RenderPassBufferHandle handle, BufferUsageBit usage, PtrSize offset = 0, PtrSize range = MAX_PTR_SIZE)
		: m_buffer({handle, usage, offset, range})
		, m_isTexture(false)
	{
	}

private:
	Bool8 m_isTexture;
};

/// Describes how the render target will be used inside a graphics render pass.
class RenderTargetReference
{
public:
	RenderTargetHandle m_renderTargetHandle;

	TextureSurfaceInfo m_surface;
	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;
	ClearValue m_clearValue;

	AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::STORE;

	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE; ///< Relevant only for depth stencil textures.
};

/// The base of compute/transfer and graphics renderpasses
class RenderPassBase
{
	friend class RenderGraph;

public:
	virtual ~RenderPassBase()
	{
		m_name.destroy(m_alloc); // To avoid the assertion
		m_consumers.destroy(m_alloc);
		m_producers.destroy(m_alloc);
	}

	void setWork(RenderPassWorkCallback callback, void* userData, U32 secondLeveCmdbCount)
	{
		m_callback = callback;
		m_userData = userData;
		m_secondLevelCmdbsCount = secondLeveCmdbCount;
	}

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

	RenderPassBase(Type t)
		: m_type(t)
	{
	}

	void setName(CString name)
	{
		m_name.create(m_alloc, (name.isEmpty()) ? "N/A" : name);
	}
};

/// XXX
class GraphicsRenderPassInfo : public RenderPassBase
{
	friend class RenderGraphDescription;

public:
	GraphicsRenderPassInfo()
		: RenderPassBase(Type::GRAPHICS)
	{
	}

	~GraphicsRenderPassInfo()
	{
	}

	void setColorRenderTarget(U32 location, const RenderTargetReference& ref)
	{
		m_colorAttachments[location] = ref;
		m_colorAttachmentCount = location + 1;
	}

	void setDepthStencilRenderTarget(const RenderTargetReference& ref)
	{
		m_depthStencilAttachment = ref;
	}

private:
	Array<RenderTargetReference, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentCount = 0;
	RenderTargetReference m_depthStencilAttachment;
};

/// XXX
class NoGraphicsRenderPassInfo : private RenderPassBase
{
private:
	NoGraphicsRenderPassInfo()
		: RenderPassBase(Type::NO_GRAPHICS)
	{
	}
};

/// XXX
class RenderTarget
{
	friend class RenderGraphDescription;
	friend class RenderGraph;

private:
	TextureInitInfo m_initInfo;
	TexturePtr m_importedTex;
	TextureUsageBit m_usage;
	Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
};

/// XXX
class RenderGraphDescription
{
	friend class RenderGraph;

public:
	RenderGraphDescription(StackAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~RenderGraphDescription()
	{
		for(RenderPassBase* pass : m_passes)
		{
			m_alloc.deleteInstance(pass);
		}
		m_passes.destroy(m_alloc);

		m_renderTargets.destroy(m_alloc);
	}

	/// Create a new graphics render pass.
	GraphicsRenderPassInfo& newGraphicsRenderPass(CString name)
	{
		GraphicsRenderPassInfo* pass = m_alloc.newInstance<GraphicsRenderPassInfo>();
		pass->m_alloc = m_alloc;
		pass->setName(name);
		m_passes.emplaceBack(m_alloc, pass);
		return *pass;
	}

	/// XXX
	U32 importRenderTarget(CString name, TexturePtr tex, TextureUsageBit usage)
	{
		RenderTarget rt;
		rt.m_importedTex = tex;
		rt.m_usage = usage;
		const U len = name.getLength();
		if(len)
		{
			ANKI_ASSERT(len <= MAX_GR_OBJECT_NAME_LENGTH);
			memcpy(&rt.m_name[0], &name[0], len);
		}
		rt.m_name[len] = '\0';

		m_renderTargets.emplaceBack(m_alloc, rt);
		return m_renderTargets.getSize() - 1;
	}

	/// XXX
	U32 newRenderTarget(CString name, const TextureInitInfo& initInf)
	{
		RenderTarget rt;
		rt.m_initInfo = initInf;
		rt.m_usage = TextureUsageBit::NONE;
		const U len = name.getLength();
		if(len)
		{
			ANKI_ASSERT(len <= MAX_GR_OBJECT_NAME_LENGTH);
			memcpy(&rt.m_name[0], &name[0], len);
		}
		rt.m_name[len] = '\0';
		m_renderTargets.emplaceBack(m_alloc, rt);
		return m_renderTargets.getSize() - 1;
	}

private:
	StackAllocator<U8> m_alloc;
	DynamicArray<RenderPassBase*> m_passes;
	DynamicArray<RenderTarget> m_renderTargets;
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
	void compileNewGraph(const RenderGraphDescription& descr);
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
	TexturePtr getTexture(RenderTargetHandle handle);
	BufferPtr getBuffer(RenderPassBufferHandle handle);
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

	HashMap<TextureInitInfo, RenderTargetCacheEntry> m_renderTargetCache; ///< Non-imported render targets.
	HashMap<FramebufferInitInfo, FramebufferPtr> m_framebufferCache;

	// Forward declarations
	class BakeContext;
	class Pass;
	class Batch;
	class RT;
	class Barrier;

	TexturePtr getOrCreateRenderTarget(const TextureInitInfo& initInf);

	static Bool passADependsOnB(BakeContext& ctx, const RenderPassBase& a, const RenderPassBase& b);
	static Bool passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx);

	void setBatchBarriers(BakeContext& ctx) const;

	/// Dump the dependency graph into a file.
	ANKI_USE_RESULT Error dumpDependencyDotFile(const BakeContext& ctx, CString path) const;

	static StringAuto textureUsageToStr(const BakeContext& ctx, TextureUsageBit usage);
};
/// @}

} // end namespace anki
