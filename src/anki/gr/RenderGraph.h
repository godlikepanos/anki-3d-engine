// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/gr/Enums.h>

namespace anki
{

/// @addtogroup gr
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
	RenderGraphHandle importRenderTarget(CString name, TexturePtr tex);

	RenderGraphHandle newRenderTarget(CString name, const TextureInitInfo& texInf);

	RenderGraphHandle importBuffer(CString name, BufferPtr buff);

	RenderGraphHandle newBuffer(CString name, PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);

	void registerRenderPass(CString name,
		WeakArray<RenderTargetInfo> colorAttachments,
		RenderTargetInfo depthStencilAttachment,
		WeakArray<RenderGraphDependency> consumers,
		WeakArray<RenderGraphDependency> producers,
		void* userData,
		RenderGraphWorkCallback callback,
		U32 secondLevelCmdbsCount);

	/// For compute or other work (mipmap gen).
	void registerNonRenderPass(CString name,
		WeakArray<RenderGraphDependency> consumers,
		WeakArray<RenderGraphDependency> producers,
		void* userData,
		RenderGraphWorkCallback callback);

	TexturePtr getTexture(RenderGraphHandle handle);

	BufferPtr getBuffer(RenderGraphHandle handle);

	void run();

	void reset();
};
/// @}

} // end namespace anki
