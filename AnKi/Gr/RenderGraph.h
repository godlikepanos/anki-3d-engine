// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/TimestampQuery.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/Function.h>

namespace anki {

// Forward
class RenderGraph;
class RenderGraphBuilder;

/// @addtogroup graphics
/// @{

/// @name RenderGraph constants
/// @{
constexpr U32 kMaxRenderGraphPasses = 512;
constexpr U32 kMaxRenderGraphRenderTargets = 128; ///< Max imported or not render targets in RenderGraph.
constexpr U32 kMaxRenderGraphBuffers = 256;
constexpr U32 kMaxRenderGraphAccelerationStructures = 32;
/// @}

/// Render target handle used in the RenderGraph.
/// @memberof RenderGraphBuilder
class RenderGraphGrObjectHandle
{
	friend class RenderPassDependency;
	friend class RenderGraphBuilder;
	friend class RenderGraph;
	friend class RenderPassBase;

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
		return m_idx != kMaxU32;
	}

private:
	U32 m_idx = kMaxU32;
};

/// Render target (TexturePtr) handle.
/// @memberof RenderGraphBuilder
class RenderTargetHandle : public RenderGraphGrObjectHandle
{
};

/// BufferPtr handle.
/// @memberof RenderGraphBuilder
class BufferHandle : public RenderGraphGrObjectHandle
{
};

/// AccelerationStructurePtr handle.
/// @memberof RenderGraphBuilder
class AccelerationStructureHandle : public RenderGraphGrObjectHandle
{
};

/// Describes the render target.
/// @memberof RenderGraphBuilder
class RenderTargetDesc : public TextureInitInfo
{
	friend class RenderGraphBuilder;

public:
	RenderTargetDesc()
	{
	}

	RenderTargetDesc(CString name)
		: TextureInitInfo(name)
	{
	}

	/// Create an internal hash.
	void bake()
	{
		ANKI_ASSERT(m_hash == 0);
		ANKI_ASSERT(m_usage == TextureUsageBit::kNone && "No need to supply the usage. RenderGraph will find out");
		m_hash = computeHash();
		// Append the name to the hash because there might be RTs with the same properties and thus the same hash. We can't have different Rt
		// descriptors with the same hash
		ANKI_ASSERT(getName().getLength() > 0);
		m_hash = appendHash(getName().cstr(), getName().getLength(), m_hash);
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
	CommandBuffer* m_commandBuffer = nullptr;

	void getBufferState(BufferHandle handle, Buffer*& buff, PtrSize& offset, PtrSize& range) const;

	void getRenderTargetState(RenderTargetHandle handle, const TextureSubresourceDesc& subresource, Texture*& tex) const;

	TextureView createTextureView(RenderTargetHandle handle, const TextureSubresourceDesc& subresource) const
	{
		Texture* tex;
		getRenderTargetState(handle, subresource, tex);
		return TextureView(tex, subresource);
	}

	void bindSrv(U32 reg, U32 space, RenderTargetHandle handle, const TextureSubresourceDesc& subresource = TextureSubresourceDesc::all())
	{
		Texture* tex;
		getRenderTargetState(handle, subresource, tex);
		m_commandBuffer->bindSrv(reg, space, TextureView(tex, subresource));
	}

	void bindUav(U32 reg, U32 space, RenderTargetHandle handle, const TextureSubresourceDesc& subresource = TextureSubresourceDesc::all())
	{
		Texture* tex;
		getRenderTargetState(handle, subresource, tex);
		m_commandBuffer->bindUav(reg, space, TextureView(tex, subresource));
	}

	void bindSrv(U32 reg, U32 space, BufferHandle handle)
	{
		Buffer* buff;
		PtrSize offset, range;
		getBufferState(handle, buff, offset, range);
		m_commandBuffer->bindSrv(reg, space, BufferView(buff, offset, range));
	}

	void bindUav(U32 reg, U32 space, BufferHandle handle)
	{
		Buffer* buff;
		PtrSize offset, range;
		getBufferState(handle, buff, offset, range);
		m_commandBuffer->bindUav(reg, space, BufferView(buff, offset, range));
	}

	void bindSrv(U32 reg, U32 space, AccelerationStructureHandle handle);

	void bindConstantBuffer(U32 reg, U32 space, BufferHandle handle)
	{
		Buffer* buff;
		PtrSize offset, range;
		getBufferState(handle, buff, offset, range);
		m_commandBuffer->bindConstantBuffer(reg, space, BufferView(buff, offset, range));
	}

private:
	const RenderGraph* m_rgraph ANKI_DEBUG_CODE(= nullptr);
	U32 m_passIdx ANKI_DEBUG_CODE(= kMaxU32);
	U32 m_batchIdx ANKI_DEBUG_CODE(= kMaxU32);

	Texture& getTexture(RenderTargetHandle handle) const;
};

/// RenderGraph pass dependency.
/// @memberof RenderGraphBuilder
class RenderPassDependency
{
	friend class RenderGraph;
	friend class RenderPassBase;

public:
	/// Dependency to a texture subresource.
	RenderPassDependency(RenderTargetHandle handle, TextureUsageBit usage, const TextureSubresourceDesc& subresource)
		: m_texture({handle, usage, subresource})
		, m_type(Type::kTexture)
	{
		ANKI_ASSERT(handle.isValid());
	}

	RenderPassDependency(BufferHandle handle, BufferUsageBit usage)
		: m_buffer({handle, usage})
		, m_type(Type::kBuffer)
	{
		ANKI_ASSERT(handle.isValid());
	}

	RenderPassDependency(AccelerationStructureHandle handle, AccelerationStructureUsageBit usage)
		: m_as({handle, usage})
		, m_type(Type::kAccelerationStructure)
	{
		ANKI_ASSERT(handle.isValid());
	}

private:
	class TextureInfo
	{
	public:
		RenderTargetHandle m_handle;
		TextureUsageBit m_usage;
		TextureSubresourceDesc m_subresource = TextureSubresourceDesc::all();
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
		kBuffer,
		kTexture,
		kAccelerationStructure
	};

	Type m_type;
};

/// The base of compute/transfer and graphics renderpasses for RenderGraph.
/// @memberof RenderGraphBuilder
class RenderPassBase
{
	friend class RenderGraph;
	friend class RenderGraphBuilder;

public:
	template<typename TFunc>
	void setWork(TFunc func)
	{
		m_callback = {func, m_rtDeps.getMemoryPool().m_pool};
	}

	void newTextureDependency(RenderTargetHandle handle, TextureUsageBit usage, const TextureSubresourceDesc& subresource)
	{
		newDependency<RenderPassDependency::Type::kTexture>(RenderPassDependency(handle, usage, subresource));
	}

	void newTextureDependency(RenderTargetHandle handle, TextureUsageBit usage, DepthStencilAspectBit aspect = DepthStencilAspectBit::kNone)
	{
		newDependency<RenderPassDependency::Type::kTexture>(RenderPassDependency(handle, usage, TextureSubresourceDesc::all(aspect)));
	}

	void newBufferDependency(BufferHandle handle, BufferUsageBit usage)
	{
		newDependency<RenderPassDependency::Type::kBuffer>(RenderPassDependency(handle, usage));
	}

	void newAccelerationStructureDependency(AccelerationStructureHandle handle, AccelerationStructureUsageBit usage)
	{
		newDependency<RenderPassDependency::Type::kAccelerationStructure>(RenderPassDependency(handle, usage));
	}

protected:
	enum class Type : U8
	{
		kGraphics,
		kNonGraphics
	};

	Type m_type;

	RenderGraphBuilder* m_descr;

	Function<void(RenderPassWorkContext&), MemoryPoolPtrWrapper<StackMemoryPool>> m_callback;

	DynamicArray<RenderPassDependency, MemoryPoolPtrWrapper<StackMemoryPool>> m_rtDeps;
	DynamicArray<RenderPassDependency, MemoryPoolPtrWrapper<StackMemoryPool>> m_buffDeps;
	DynamicArray<RenderPassDependency, MemoryPoolPtrWrapper<StackMemoryPool>> m_asDeps;

	BitSet<kMaxRenderGraphRenderTargets, U64> m_readRtMask{false};
	BitSet<kMaxRenderGraphRenderTargets, U64> m_writeRtMask{false};
	BitSet<kMaxRenderGraphBuffers, U64> m_readBuffMask{false};
	BitSet<kMaxRenderGraphBuffers, U64> m_writeBuffMask{false};
	BitSet<kMaxRenderGraphAccelerationStructures, U32> m_readAsMask{false};
	BitSet<kMaxRenderGraphAccelerationStructures, U32> m_writeAsMask{false};

	BaseString<MemoryPoolPtrWrapper<StackMemoryPool>> m_name;

	RenderPassBase(Type t, RenderGraphBuilder* descr, StackMemoryPool* pool)
		: m_type(t)
		, m_descr(descr)
		, m_rtDeps(pool)
		, m_buffDeps(pool)
		, m_asDeps(pool)
		, m_name(pool)
	{
		ANKI_ASSERT(descr && pool);
	}

	void setName(CString name)
	{
		m_name = (name.isEmpty()) ? "N/A" : name;
	}

	void fixSubresource(RenderPassDependency& dep) const;

	void validateDep(const RenderPassDependency& dep);

	/// Add a new consumer or producer dependency.
	template<RenderPassDependency::Type kType>
	void newDependency(const RenderPassDependency& dep);
};

/// Renderpass attachment info. Used in GraphicsRenderPass::setRenderpassInfo. It mirrors the RenderPass.
/// @memberof GraphicsRenderPass
class GraphicsRenderPassTargetDesc
{
public:
	RenderTargetHandle m_handle;
	TextureSubresourceDesc m_subresource = TextureSubresourceDesc::firstSurface();

	RenderTargetLoadOperation m_loadOperation = RenderTargetLoadOperation::kDontCare;
	RenderTargetStoreOperation m_storeOperation = RenderTargetStoreOperation::kStore;

	RenderTargetLoadOperation m_stencilLoadOperation = RenderTargetLoadOperation::kDontCare;
	RenderTargetStoreOperation m_stencilStoreOperation = RenderTargetStoreOperation::kStore;

	ClearValue m_clearValue;

	GraphicsRenderPassTargetDesc() = default;

	GraphicsRenderPassTargetDesc(RenderTargetHandle handle)
		: m_handle(handle)
	{
	}
};

/// A graphics render pass for RenderGraph.
/// @memberof RenderGraphBuilder
class GraphicsRenderPass : public RenderPassBase
{
	friend class RenderGraphBuilder;
	friend class RenderGraph;

public:
	GraphicsRenderPass(RenderGraphBuilder* descr, StackMemoryPool* pool)
		: RenderPassBase(Type::kGraphics, descr, pool)
	{
	}

	void setRenderpassInfo(ConstWeakArray<GraphicsRenderPassTargetDesc> colorRts, const GraphicsRenderPassTargetDesc* depthStencilRt = nullptr,
						   U32 minx = 0, U32 miny = 0, U32 width = kMaxU32, U32 height = kMaxU32, const RenderTargetHandle* vrsRt = nullptr,
						   U8 vrsRtTexelSizeX = 0, U8 vrsRtTexelSizeY = 0);

	void setRenderpassInfo(std::initializer_list<GraphicsRenderPassTargetDesc> colorRts, const GraphicsRenderPassTargetDesc* depthStencilRt = nullptr,
						   U32 minx = 0, U32 miny = 0, U32 width = kMaxU32, U32 height = kMaxU32, const RenderTargetHandle* vrsRt = nullptr,
						   U8 vrsRtTexelSizeX = 0, U8 vrsRtTexelSizeY = 0)
	{
		ConstWeakArray<GraphicsRenderPassTargetDesc> colorRtsArr(colorRts.begin(), U32(colorRts.size()));
		setRenderpassInfo(colorRtsArr, depthStencilRt, minx, miny, width, height, vrsRt, vrsRtTexelSizeX, vrsRtTexelSizeY);
	}

private:
	Array<GraphicsRenderPassTargetDesc, kMaxColorRenderTargets + 2> m_rts;
	Array<U32, 4> m_rpassRenderArea = {};
	U8 m_colorRtCount = 0;
	U8 m_vrsRtTexelSizeX = 0;
	U8 m_vrsRtTexelSizeY = 0;

	Bool hasRenderpass() const
	{
		return m_rpassRenderArea[3] != 0;
	}
};

/// A compute render pass for RenderGraph.
/// @memberof RenderGraphBuilder
class NonGraphicsRenderPass : public RenderPassBase
{
	friend class RenderGraphBuilder;

public:
	NonGraphicsRenderPass(RenderGraphBuilder* descr, StackMemoryPool* pool)
		: RenderPassBase(Type::kNonGraphics, descr, pool)
	{
	}
};

/// Builds the description of the frame's render passes and their interactions.
/// @memberof RenderGraph
class RenderGraphBuilder
{
	friend class RenderGraph;
	friend class RenderPassBase;

public:
	RenderGraphBuilder(StackMemoryPool* pool)
		: m_pool(pool)
	{
	}

	~RenderGraphBuilder();

	/// Create a new graphics render pass.
	GraphicsRenderPass& newGraphicsRenderPass(CString name);

	/// Create a new compute render pass.
	NonGraphicsRenderPass& newNonGraphicsRenderPass(CString name);

	/// Import an existing render target and let the render graph know about it's up-to-date usage.
	RenderTargetHandle importRenderTarget(Texture* tex, TextureUsageBit usage);

	/// Import an existing render target and let the render graph find it's current usage by looking at the previous frame.
	RenderTargetHandle importRenderTarget(Texture* tex);

	/// Get or create a new render target.
	RenderTargetHandle newRenderTarget(const RenderTargetDesc& initInf);

	/// Import a buffer.
	BufferHandle importBuffer(const BufferView& buff, BufferUsageBit crntUsage);

	/// Import an AS.
	AccelerationStructureHandle importAccelerationStructure(AccelerationStructure* as, AccelerationStructureUsageBit crntUsage);

	/// Gather statistics.
	void setStatisticsEnabled(Bool gather)
	{
		m_gatherStatistics = gather;
	}

private:
	class Resource
	{
	public:
		Array<char, kMaxGrObjectNameLength + 1> m_name;

		void setName(CString name)
		{
			const U len = name.getLength();
			ANKI_ASSERT(len <= kMaxGrObjectNameLength);
			strcpy(&m_name[0], (len) ? &name[0] : "unnamed");
		}
	};

	class RT : public Resource
	{
	public:
		TextureInitInfo m_initInfo;
		U64 m_hash = 0;
		TexturePtr m_importedTex;
		TextureUsageBit m_importedLastKnownUsage = TextureUsageBit::kNone;
		/// Derived by the deps of this RT and will be used to set its usage.
		TextureUsageBit m_usageDerivedByDeps = TextureUsageBit::kNone;
		Bool m_importedAndUndefinedUsage = false;
	};

	class BufferRsrc : public Resource
	{
	public:
		BufferUsageBit m_usage;
		BufferPtr m_importedBuff;
		PtrSize m_offset;
		PtrSize m_range;
	};

	class AS : public Resource
	{
	public:
		AccelerationStructurePtr m_importedAs;
		AccelerationStructureUsageBit m_usage;
	};

	StackMemoryPool* m_pool = nullptr;
	DynamicArray<RenderPassBase*, MemoryPoolPtrWrapper<StackMemoryPool>> m_passes{m_pool};
	DynamicArray<RT, MemoryPoolPtrWrapper<StackMemoryPool>> m_renderTargets{m_pool};
	DynamicArray<BufferRsrc, MemoryPoolPtrWrapper<StackMemoryPool>> m_buffers{m_pool};
	DynamicArray<AS, MemoryPoolPtrWrapper<StackMemoryPool>> m_as{m_pool};
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
/// - Command buffer creation .
/// - Render target creation (optional since textures can be imported as well).
///
/// It accepts a description of the frame's render passes (compute and graphics), compiles that description to calculate
/// dependencies and then populates command buffers with the help of multiple RenderPassWorkCallback.
class RenderGraph final : public GrObject
{
	ANKI_GR_OBJECT

	friend class RenderPassWorkContext;

public:
	static constexpr GrObjectType kClassType = GrObjectType::kRenderGraph;

	/// 1st step.
	void compileNewGraph(const RenderGraphBuilder& descr, StackMemoryPool& pool);

	/// 2nd step. Will call a number of RenderPassWorkCallback that populate command buffers and submit work.
	void recordAndSubmitCommandBuffers(FencePtr* optionalFence = nullptr);

	/// 3rd step. Reset the graph for a new frame. All previously created RenderGraphHandle are invalid after that call.
	void reset();

	/// [OPTIONAL] 4th step. Get some statistics.
	void getStatistics(RenderGraphStatistics& statistics);

private:
	static constexpr U kPeriodicCleanupEvery = 60; ///< How many frames between cleanups.

	// Forward declarations of internal classes.
	class BakeContext;
	class Pass;
	class Batch;
	class RT;
	class BufferRange;
	class AS;
	class TextureBarrier;
	class BufferBarrier;
	class ASBarrier;

	/// Render targets of the same type+size+format.
	class RenderTargetCacheEntry
	{
	public:
		GrDynamicArray<TexturePtr> m_textures;
		U32 m_texturesInUse = 0;
	};

	/// Info on imported render targets that are kept between runs.
	class ImportedRenderTargetInfo
	{
	public:
		GrDynamicArray<TextureUsageBit> m_surfOrVolLastUsages; ///< Last TextureUsageBit of the imported RT.
	};

	GrHashMap<U64, RenderTargetCacheEntry> m_renderTargetCache; ///< Non-imported render targets.
	GrHashMap<U64, ImportedRenderTargetInfo> m_importedRenderTargets;

	BakeContext* m_ctx = nullptr;
	U64 m_version = 0;

	static constexpr U kMaxBufferedTimestamps = kMaxFramesInFlight + 1;
	class
	{
	public:
		Array2d<TimestampQueryPtr, kMaxBufferedTimestamps, 2> m_timestamps;
		Array<Second, kMaxBufferedTimestamps> m_cpuStartTimes;
		U8 m_nextTimestamp = 0;
	} m_statistics;

	RenderGraph(CString name);

	~RenderGraph();

	[[nodiscard]] static RenderGraph* newInstance();

	BakeContext* newContext(const RenderGraphBuilder& descr, StackMemoryPool& pool);
	void initRenderPassesAndSetDeps(const RenderGraphBuilder& descr);
	void initBatches();
	void initGraphicsPasses(const RenderGraphBuilder& descr);
	void setBatchBarriers(const RenderGraphBuilder& descr);
	/// Switching from compute to graphics and the opposite in the same queue is not great for some GPUs (nVidia)
	void minimizeSubchannelSwitches();
	void sortBatchPasses();

	TexturePtr getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash);

	/// Every N number of frames clean unused cached items.
	void periodicCleanup();

	ANKI_HOT static Bool passADependsOnB(const RenderPassBase& a, const RenderPassBase& b);

	static Bool passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx);

	void setTextureBarrier(Batch& batch, const RenderPassDependency& consumer);

	template<typename TFunc>
	static void iterateSurfsOrVolumes(const Texture& tex, const TextureSubresourceDesc& subresource, TFunc func);

	void getCrntUsage(RenderTargetHandle handle, U32 batchIdx, const TextureSubresourceDesc& subresource, TextureUsageBit& usage) const;

	/// @name Dump the dependency graph into a file.
	/// @{
	Error dumpDependencyDotFile(const RenderGraphBuilder& descr, const BakeContext& ctx, CString path) const;
	static GrString textureUsageToStr(StackMemoryPool& pool, TextureUsageBit usage);
	static GrString bufferUsageToStr(StackMemoryPool& pool, BufferUsageBit usage);
	static GrString asUsageToStr(StackMemoryPool& pool, AccelerationStructureUsageBit usage);
	/// @}

	Texture& getTexture(RenderTargetHandle handle) const;
	void getCachedBuffer(BufferHandle handle, Buffer*& buff, PtrSize& offset, PtrSize& range) const;
	AccelerationStructure* getAs(AccelerationStructureHandle handle) const;
};
/// @}

} // end namespace anki

#include <AnKi/Gr/RenderGraph.inl.h>
