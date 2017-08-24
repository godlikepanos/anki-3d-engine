// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/gr/Enums.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// XXX
using RenderGraphHandle = U64;

/// XXX
using RenderGraphWorkCallback = void (*)(
	void* userData, CommandBufferPtr cmdb, U32 secondLevelCmdbIdx, U32 secondLevelCmdbCount);

/// XXX
class RenderGraphDependency
{
public:
	RenderGraphHandle m_handle;
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
class RenderGraph : public NonCopyable
{
public:
	RenderGraph(GrManager* gr);

	~RenderGraph();

	/// @name 1st step methods
	/// @{
	RenderGraphHandle importRenderTarget(CString name, TexturePtr tex);

	RenderGraphHandle newRenderTarget(CString name, const TextureInitInfo& texInf);

	RenderGraphHandle importBuffer(CString name, BufferPtr buff);

	RenderGraphHandle newBuffer(CString name, PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);

	void registerRenderPass(CString name,
		WeakArray<RenderTargetInfo> colorAttachments,
		RenderTargetInfo depthStencilAttachment,
		WeakArray<RenderGraphDependency> consumers,
		WeakArray<RenderGraphDependency> producers,
		RenderGraphWorkCallback callback,
		void* userData,
		U32 secondLevelCmdbsCount);

	/// For compute or other work (mipmap gen).
	void registerNonRenderPass(CString name,
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
	GrManager* m_gr;

	class Cache
	{
	public:
		HashMap<TextureInitInfo, TexturePtr> m_renderTargets; ///< Non-imported render targets.
		HashMap<FramebufferInitInfo, FramebufferPtr> m_framebuffers;
	} m_cache;

	class RenderTarget
	{
	public:
		TexturePtr m_tex;
		Bool8 m_imported = false;
		Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
	};

	DynamicArray<RenderTarget> m_renderTargets;
	RenderGraphHandle m_lastRtHandle = 0;

	class Pass
	{
	public:
		Pass* m_next = nullptr;
		FramebufferPtr m_framebuffer;
		Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
	};

	DynamicArray<Pass> m_passes;
	RenderGraphHandle m_lastPassHandle = 0;
};
/// @}

} // end namespace anki
