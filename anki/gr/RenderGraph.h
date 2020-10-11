// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/gr/TimestampQuery.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/AccelerationStructure.h>
#include <anki/util/HashMap.h>
#include <anki/util/BitSet.h>
#include <anki/util/WeakArray.h>

namespace anki
{

// Forward
class RenderGraph;
class RenderGraphDescription;

/// @addtogroup graphics
/// @{

/// @name RenderGraph constants
/// @{
constexpr U32 MAX_RENDER_GRAPH_PASSES = 128;
constexpr U32 MAX_RENDER_GRAPH_RENDER_TARGETS = 64; ///< Max imported or not render targets in RenderGraph.
constexpr U32 MAX_RENDER_GRAPH_BUFFERS = 64;
constexpr U32 MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES = 32;
/// @}

/// Render target handle used in the RenderGraph.
/// @memberof RenderGraphDescription
class RenderGraphGrObjectHandle
{
	friend class RenderPassDependency;
	friend class RenderGraphDescription;
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	Bool operator==(const RenderGraphGrObjectHandle& b) const
	{
		return m_idx == b.m_idx;
	}

	Bool operator!=(const RenderGraphGrObjectHandle& b) const
	{
		return m_idx != b.m_idx;
	}

	Bool isValid() const
	{
		return m_idx != MAX_U32;
	}

private:
	U32 m_idx = MAX_U32;
};

/// Render target (TexturePtr) handle.
/// @memberof RenderGraphDescription
class RenderTargetHandle : public RenderGraphGrObjectHandle
{
};

/// BufferPtr handle.
/// @memberof RenderGraphDescription
class BufferHandle : public RenderGraphGrObjectHandle
{
};

/// AccelerationStructurePtr handle.
/// @memberof RenderGraphDescription
class AccelerationStructureHandle : public RenderGraphGrObjectHandle
{
};

/// Describes the render target.
/// @memberof RenderGraphDescription
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
/// @memberof RenderGraph
class RenderPassWorkContext
{
	friend class RenderGraph;

public:
	void* m_userData ANKI_DEBUG_CODE(= nullptr); ///< The userData passed in RenderPassDescriptionBase::setWork
	CommandBufferPtr m_commandBuffer;
	U32 m_currentSecondLevelCommandBufferIndex ANKI_DEBUG_CODE(= 0);
	U32 m_secondLevelCommandBufferCount ANKI_DEBUG_CODE(= 0);

	void getBufferState(BufferHandle handle, BufferPtr& buff) const;

	void getRenderTargetState(RenderTargetHandle handle, const TextureSubresourceInfo& subresource, TexturePtr& tex,
							  TextureUsageBit& usage) const;

	/// Convenience method.
	void bindTextureAndSampler(U32 set, U32 binding, RenderTargetHandle handle,
							   const TextureSubresourceInfo& subresource, const SamplerPtr& sampler)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		getRenderTargetState(handle, subresource, tex, usage);
		TextureViewInitInfo viewInit(tex, subresource, "TmpRenderGraph");
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindTextureAndSampler(set, binding, view, sampler, usage);
	}

	/// Convenience method.
	void bindTexture(U32 set, U32 binding, RenderTargetHandle handle, const TextureSubresourceInfo& subresource)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		getRenderTargetState(handle, subresource, tex, usage);
		TextureViewInitInfo viewInit(tex, subresource, "TmpRenderGraph");
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindTexture(set, binding, view, usage);
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

	/// Convenience method to bind the whole texture as color.
	void bindColorTexture(U32 set, U32 binding, RenderTargetHandle handle, U32 arrayIdx = 0)
	{
		TexturePtr tex = getTexture(handle);
		TextureViewInitInfo viewInit(tex); // Use the whole texture
		TextureUsageBit usage;
		getRenderTargetState(handle, viewInit, tex, usage);
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindTexture(set, binding, view, usage, arrayIdx);
	}

	/// Convenience method.
	void bindImage(U32 set, U32 binding, RenderTargetHandle handle, const TextureSubresourceInfo& subresource,
				   U32 arrayIdx = 0)
	{
		TexturePtr tex;
		TextureUsageBit usage;
		getRenderTargetState(handle, subresource, tex, usage);
		TextureViewInitInfo viewInit(tex, subresource, "TmpRenderGraph");
		TextureViewPtr view = m_commandBuffer->getManager().newTextureView(viewInit);
		m_commandBuffer->bindImage(set, binding, view, arrayIdx);
	}

	/// Convenience method.
	void bindStorageBuffer(U32 set, U32 binding, BufferHandle handle)
	{
		BufferPtr buff;
		getBufferState(handle, buff);
		m_commandBuffer->bindStorageBuffer(set, binding, buff, 0, MAX_PTR_SIZE);
	}

	/// Convenience method.
	void bindUniformBuffer(U32 set, U32 binding, BufferHandle handle)
	{
		BufferPtr buff;
		getBufferState(handle, buff);
		m_commandBuffer->bindUniformBuffer(set, binding, buff, 0, MAX_PTR_SIZE);
	}

	/// Convenience method.
	void bindAccelerationStructure(U32 set, U32 binding, AccelerationStructureHandle handle);

private:
	const RenderGraph* m_rgraph ANKI_DEBUG_CODE(= nullptr);
	U32 m_passIdx ANKI_DEBUG_CODE(= MAX_U32);
	U32 m_batchIdx ANKI_DEBUG_CODE(= MAX_U32);

	TexturePtr getTexture(RenderTargetHandle handle) const;
};

/// Work callback for a RenderGraph pass.
/// @memberof RenderGraphDescription
using RenderPassWorkCallback = void (*)(RenderPassWorkContext& ctx);

/// RenderGraph pass dependency.
/// @memberof RenderGraphDescription
class RenderPassDependency
{
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	/// Dependency to a texture subresource.
	RenderPassDependency(RenderTargetHandle handle, TextureUsageBit usage, const TextureSubresourceInfo& subresource)
		: m_texture({handle, usage, subresource})
		, m_type(Type::TEXTURE)
	{
		ANKI_ASSERT(handle.isValid());
	}

	/// Dependency to the whole texture.
	RenderPassDependency(RenderTargetHandle handle, TextureUsageBit usage,
						 DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE)
		: m_texture({handle, usage, TextureSubresourceInfo()})
		, m_type(Type::TEXTURE)
	{
		ANKI_ASSERT(handle.isValid());
		m_texture.m_subresource.m_mipmapCount = MAX_U32; // Mark it as "whole texture"
		m_texture.m_subresource.m_depthStencilAspect = aspect;
	}

	RenderPassDependency(BufferHandle handle, BufferUsageBit usage)
		: m_buffer({handle, usage})
		, m_type(Type::BUFFER)
	{
		ANKI_ASSERT(handle.isValid());
	}

	RenderPassDependency(AccelerationStructureHandle handle, AccelerationStructureUsageBit usage)
		: m_as({handle, usage})
		, m_type(Type::ACCELERATION_STRUCTURE)
	{
		ANKI_ASSERT(handle.isValid());
	}

private:
	class TextureInfo
	{
	public:
		RenderTargetHandle m_handle;
		TextureUsageBit m_usage;
		TextureSubresourceInfo m_subresource;
	};

	class BufferInfo
	{
	public:
		BufferHandle m_handle;
		BufferUsageBit m_usage;
	};

	class ASInfo
	{
	public:
		AccelerationStructureHandle m_handle;
		AccelerationStructureUsageBit m_usage;
	};

	union
	{
		TextureInfo m_texture;
		BufferInfo m_buffer;
		ASInfo m_as;
	};

	enum class Type : U8
	{
		BUFFER,
		TEXTURE,
		ACCELERATION_STRUCTURE
	};

	Type m_type;
};

/// The base of compute/transfer and graphics renderpasses for RenderGraph.
/// @memberof RenderGraphDescription
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
		m_asDeps.destroy(m_alloc);
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
	DynamicArray<RenderPassDependency> m_asDeps;

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> m_readRtMask{false};
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> m_writeRtMask{false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> m_readBuffMask{false};
	BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> m_writeBuffMask{false};
	BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> m_readAsMask{false};
	BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> m_writeAsMask{false};

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
/// @memberof RenderGraphDescription
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
/// @memberof RenderGraphDescription
class GraphicsRenderPassDescription : public RenderPassDescriptionBase
{
	friend class RenderGraphDescription;
	friend class RenderGraph;
	template<typename, typename>
	friend class GenericPoolAllocator;

public:
	void setFramebufferInfo(const FramebufferDescription& fbInfo,
							ConstWeakArray<RenderTargetHandle> colorRenderTargetHandles,
							RenderTargetHandle depthStencilRenderTargetHandle, U32 minx = 0, U32 miny = 0,
							U32 maxx = MAX_U32, U32 maxy = MAX_U32);

	void setFramebufferInfo(const FramebufferDescription& fbInfo,
							std::initializer_list<RenderTargetHandle> colorRenderTargetHandles,
							RenderTargetHandle depthStencilRenderTargetHandle, U32 minx = 0, U32 miny = 0,
							U32 maxx = MAX_U32, U32 maxy = MAX_U32);

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
/// @memberof RenderGraphDescription
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
/// @memberof RenderGraph
class RenderGraphDescription
{
	friend class RenderGraph;
	friend class RenderPassDescriptionBase;

public:
	RenderGraphDescription(const StackAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	~RenderGraphDescription();

	/// Create a new graphics render pass.
	GraphicsRenderPassDescription& newGraphicsRenderPass(CString name);

	/// Create a new compute render pass.
	ComputeRenderPassDescription& newComputeRenderPass(CString name);

	/// Import an existing render target and let the render graph know about it's up-to-date usage.
	RenderTargetHandle importRenderTarget(TexturePtr tex, TextureUsageBit usage);

	/// Import an existing render target and let the render graph find it's current usage by looking at the previous
	/// frame.
	RenderTargetHandle importRenderTarget(TexturePtr tex);

	/// Get or create a new render target.
	RenderTargetHandle newRenderTarget(const RenderTargetDescription& initInf);

	/// Import a buffer.
	BufferHandle importBuffer(BufferPtr buff, BufferUsageBit usage);

	/// Import an AS.
	AccelerationStructureHandle importAccelerationStructure(AccelerationStructurePtr as,
															AccelerationStructureUsageBit usage);

	/// Gather statistics.
	void setStatisticsEnabled(Bool gather)
	{
		m_gatherStatistics = gather;
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
		TextureUsageBit m_importedLastKnownUsage = TextureUsageBit::NONE;
		/// Derived by the deps of this RT and will be used to set its usage.
		TextureUsageBit m_usageDerivedByDeps = TextureUsageBit::NONE;
		Bool m_importedAndUndefinedUsage = false;
	};

	class Buffer : public Resource
	{
	public:
		BufferUsageBit m_usage;
		BufferPtr m_importedBuff;
	};

	class AS : public Resource
	{
	public:
		AccelerationStructurePtr m_importedAs;
		AccelerationStructureUsageBit m_usage;
	};

	StackAllocator<U8> m_alloc;
	DynamicArray<RenderPassDescriptionBase*> m_passes;
	DynamicArray<RT> m_renderTargets;
	DynamicArray<Buffer> m_buffers;
	DynamicArray<AS> m_as;
	Bool m_gatherStatistics = false;
};

/// Statistics.
/// @memberof RenderGraph
class RenderGraphStatistics
{
public:
	Second m_gpuTime; ///< Time spent in the GPU.
	Second m_cpuStartTime; ///< Time the work was submited from the CPU (almost)
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

	/// @name 5th step methods [OPTIONAL]
	/// @{

	/// Get some statistics.
	void getStatistics(RenderGraphStatistics& statistics) const;
	/// @}

private:
	static constexpr U PERIODIC_CLEANUP_EVERY = 60; ///< How many frames between cleanups.

	// Forward declarations of internal classes.
	class BakeContext;
	class Pass;
	class Batch;
	class RT;
	class Buffer;
	class AS;
	class TextureBarrier;
	class BufferBarrier;
	class ASBarrier;

	/// Render targets of the same type+size+format.
	class RenderTargetCacheEntry
	{
	public:
		DynamicArray<TexturePtr> m_textures;
		U32 m_texturesInUse = 0;
	};

	/// Info on imported render targets that are kept between runs.
	class ImportedRenderTargetInfo
	{
	public:
		DynamicArray<TextureUsageBit> m_surfOrVolLastUsages; ///< Last TextureUsageBit of the imported RT.
	};

	HashMap<U64, RenderTargetCacheEntry> m_renderTargetCache; ///< Non-imported render targets.
	HashMap<U64, FramebufferPtr> m_fbCache; ///< Framebuffer cache.
	HashMap<U64, ImportedRenderTargetInfo> m_importedRenderTargets;

	BakeContext* m_ctx = nullptr;
	U64 m_version = 0;

	static constexpr U MAX_TIMESTAMPS_BUFFERED = MAX_FRAMES_IN_FLIGHT + 1;
	class
	{
	public:
		Array<TimestampQueryPtr, MAX_TIMESTAMPS_BUFFERED * 2> m_timestamps;
		Array<Second, MAX_TIMESTAMPS_BUFFERED> m_cpuStartTimes;
		U8 m_nextTimestamp = 0;
	} m_statistics;

	RenderGraph(GrManager* manager, CString name);

	~RenderGraph();

	static ANKI_USE_RESULT RenderGraph* newInstance(GrManager* manager);

	BakeContext* newContext(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void initBatches();
	void initGraphicsPasses(const RenderGraphDescription& descr, StackAllocator<U8>& alloc);
	void setBatchBarriers(const RenderGraphDescription& descr);

	TexturePtr getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash);
	FramebufferPtr getOrCreateFramebuffer(const FramebufferDescription& fbDescr, const RenderTargetHandle* rtHandles,
										  CString name, Bool& drawsToPresentableTex);

	/// Every N number of frames clean unused cached items.
	void periodicCleanup();

	ANKI_HOT static Bool passADependsOnB(const RenderPassDescriptionBase& a, const RenderPassDescriptionBase& b);

	static Bool overlappingTextureSubresource(const TextureSubresourceInfo& suba, const TextureSubresourceInfo& subb);

	static Bool passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx);

	void setTextureBarrier(Batch& batch, const RenderPassDependency& consumer);

	template<typename TFunc>
	static void iterateSurfsOrVolumes(const TexturePtr& tex, const TextureSubresourceInfo& subresource, TFunc func);

	void getCrntUsage(RenderTargetHandle handle, U32 batchIdx, const TextureSubresourceInfo& subresource,
					  TextureUsageBit& usage) const;

	/// @name Dump the dependency graph into a file.
	/// @{
	ANKI_USE_RESULT Error dumpDependencyDotFile(const RenderGraphDescription& descr, const BakeContext& ctx,
												CString path) const;
	static StringAuto textureUsageToStr(StackAllocator<U8>& alloc, TextureUsageBit usage);
	static StringAuto bufferUsageToStr(StackAllocator<U8>& alloc, BufferUsageBit usage);
	static StringAuto asUsageToStr(StackAllocator<U8>& alloc, AccelerationStructureUsageBit usage);
	/// @}

	TexturePtr getTexture(RenderTargetHandle handle) const;
	BufferPtr getBuffer(BufferHandle handle) const;
	AccelerationStructurePtr getAs(AccelerationStructureHandle handle) const;
};
/// @}

} // end namespace anki

#include <anki/gr/RenderGraph.inl.h>
