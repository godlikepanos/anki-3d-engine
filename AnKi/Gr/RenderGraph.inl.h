// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/RenderGraph.h>

namespace anki {

inline void RenderPassWorkContext::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructureHandle handle)
{
	m_commandBuffer->bindAccelerationStructure(set, binding, m_rgraph->getAs(handle));
}

inline void RenderPassWorkContext::getBufferState(BufferHandle handle, Buffer*& buff, PtrSize& offset, PtrSize& range) const
{
	m_rgraph->getCachedBuffer(handle, buff, offset, range);
}

inline void RenderPassWorkContext::getRenderTargetState(RenderTargetHandle handle, const TextureSubresourceInfo& subresource, Texture*& tex) const
{
	TextureUsageBit usage;
	m_rgraph->getCrntUsage(handle, m_batchIdx, subresource, usage);
	tex = &m_rgraph->getTexture(handle);
}

inline Texture& RenderPassWorkContext::getTexture(RenderTargetHandle handle) const
{
	return m_rgraph->getTexture(handle);
}

inline void RenderPassDescriptionBase::fixSubresource(RenderPassDependency& dep) const
{
	ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::kTexture);

	TextureSubresourceInfo& subresource = dep.m_texture.m_subresource;
	const Bool wholeTexture = subresource.m_mipmapCount == kMaxU32;
	const RenderGraphDescription::RT& rt = m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx];
	if(wholeTexture)
	{
		ANKI_ASSERT(subresource.m_firstFace == 0);
		ANKI_ASSERT(subresource.m_firstMipmap == 0);
		ANKI_ASSERT(subresource.m_firstLayer == 0);

		if(rt.m_importedTex)
		{
			subresource.m_faceCount = textureTypeIsCube(rt.m_importedTex->getTextureType()) ? 6 : 1;
			subresource.m_mipmapCount = rt.m_importedTex->getMipmapCount();
			subresource.m_layerCount = rt.m_importedTex->getLayerCount();
		}
		else
		{
			subresource.m_faceCount = textureTypeIsCube(rt.m_initInfo.m_type) ? 6 : 1;
			subresource.m_mipmapCount = rt.m_initInfo.m_mipmapCount;
			subresource.m_layerCount = rt.m_initInfo.m_layerCount;
		}
	}

	if(subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone)
	{
		const Bool imported = rt.m_importedTex.isCreated();
		if(imported)
		{
			subresource.m_depthStencilAspect = rt.m_importedTex->getDepthStencilAspect();
		}
		else if(!imported && getFormatInfo(rt.m_initInfo.m_format).isDepthStencil())
		{
			subresource.m_depthStencilAspect = getFormatInfo(rt.m_initInfo.m_format).m_depthStencil;
		}
	}

	ANKI_ASSERT(dep.m_texture.m_subresource.m_firstMipmap + dep.m_texture.m_subresource.m_mipmapCount
				<= ((rt.m_importedTex) ? rt.m_importedTex->getMipmapCount() : rt.m_initInfo.m_mipmapCount));
}

inline void RenderPassDescriptionBase::validateDep(const RenderPassDependency& dep)
{
	// Validate dep
	if(dep.m_type == RenderPassDependency::Type::kTexture)
	{
		[[maybe_unused]] const TextureUsageBit usage = dep.m_texture.m_usage;
		if(m_type == Type::kGraphics)
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::kAllCompute));
		}
		else
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::kAllGraphics));
		}

		ANKI_ASSERT(!!(usage & TextureUsageBit::kAllRead) || !!(usage & TextureUsageBit::kAllWrite));
	}
	else if(dep.m_type == RenderPassDependency::Type::kBuffer)
	{
		[[maybe_unused]] const BufferUsageBit usage = dep.m_buffer.m_usage;
		if(m_type == Type::kGraphics)
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::kAllCompute));
		}
		else
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::kAllGraphics));
		}

		ANKI_ASSERT(!!(usage & BufferUsageBit::kAllRead) || !!(usage & BufferUsageBit::kAllWrite));
	}
	else
	{
		ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::kAccelerationStructure);
		if(m_type == Type::kGraphics)
		{
			ANKI_ASSERT(!(dep.m_as.m_usage & ~AccelerationStructureUsageBit::kAllGraphics));
		}
		else
		{
			ANKI_ASSERT(!(dep.m_as.m_usage & AccelerationStructureUsageBit::kAllGraphics));
		}
	}
}

template<RenderPassDependency::Type kType>
inline void RenderPassDescriptionBase::newDependency(const RenderPassDependency& dep)
{
	ANKI_ASSERT(kType == dep.m_type);
	validateDep(dep);

	if(kType == RenderPassDependency::Type::kTexture)
	{
		m_rtDeps.emplaceBack(dep);
		fixSubresource(m_rtDeps.getBack());

		if(!!(dep.m_texture.m_usage & TextureUsageBit::kAllRead))
		{
			m_readRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		if(!!(dep.m_texture.m_usage & TextureUsageBit::kAllWrite))
		{
			m_writeRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		// Try to derive the usage by that dep
		m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx].m_usageDerivedByDeps |= dep.m_texture.m_usage;

		// Checks
#if ANKI_ASSERTIONS_ENABLED
		const RenderGraphDescription::RT& rt = m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx];
		if((!rt.m_importedTex.isCreated() && !!getFormatInfo(rt.m_initInfo.m_format).m_depthStencil)
		   || (rt.m_importedTex.isCreated() && !!rt.m_importedTex->getDepthStencilAspect()))
		{
			ANKI_ASSERT(!!m_rtDeps.getBack().m_texture.m_subresource.m_depthStencilAspect
						&& "Dependencies of depth/stencil resources should have a valid DS aspect");
		}
#endif
	}
	else if(kType == RenderPassDependency::Type::kBuffer)
	{
		ANKI_ASSERT(!!(m_descr->m_buffers[dep.m_buffer.m_handle.m_idx].m_importedBuff->getBufferUsage() & dep.m_buffer.m_usage));

		m_buffDeps.emplaceBack(dep);

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::kAllRead))
		{
			m_readBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::kAllWrite))
		{
			m_writeBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}
	}
	else
	{
		ANKI_ASSERT(kType == RenderPassDependency::Type::kAccelerationStructure);
		m_asDeps.emplaceBack(dep);

		if(!!(dep.m_as.m_usage & AccelerationStructureUsageBit::kAllRead))
		{
			m_readAsMask.set(dep.m_as.m_handle.m_idx);
		}

		if(!!(dep.m_as.m_usage & AccelerationStructureUsageBit::kAllWrite))
		{
			m_writeAsMask.set(dep.m_as.m_handle.m_idx);
		}
	}
}

inline void GraphicsRenderPassDescription::setRenderpassInfo(ConstWeakArray<RenderTargetInfo> colorRts, const RenderTargetInfo* depthStencilRt,
															 U32 minx, U32 miny, U32 width, U32 height, const RenderTargetHandle* vrsRt,
															 U8 vrsRtTexelSizeX, U8 vrsRtTexelSizeY)
{
	m_colorRtCount = U8(colorRts.getSize());
	for(U32 i = 0; i < m_colorRtCount; ++i)
	{
		m_rts[i].m_handle = colorRts[i].m_handle;
		m_rts[i].m_surface = colorRts[i].m_surface;
		m_rts[i].m_loadOperation = colorRts[i].m_loadOperation;
		m_rts[i].m_storeOperation = colorRts[i].m_storeOperation;
		m_rts[i].m_clearValue = colorRts[i].m_clearValue;
	}

	if(depthStencilRt)
	{
		m_rts[kMaxColorRenderTargets].m_handle = depthStencilRt->m_handle;
		m_rts[kMaxColorRenderTargets].m_surface = depthStencilRt->m_surface;
		ANKI_ASSERT(!!depthStencilRt->m_aspect);
		m_rts[kMaxColorRenderTargets].m_aspect = depthStencilRt->m_aspect;
		m_rts[kMaxColorRenderTargets].m_loadOperation = depthStencilRt->m_loadOperation;
		m_rts[kMaxColorRenderTargets].m_storeOperation = depthStencilRt->m_storeOperation;
		m_rts[kMaxColorRenderTargets].m_stencilLoadOperation = depthStencilRt->m_stencilLoadOperation;
		m_rts[kMaxColorRenderTargets].m_stencilStoreOperation = depthStencilRt->m_stencilStoreOperation;
		m_rts[kMaxColorRenderTargets].m_clearValue = depthStencilRt->m_clearValue;
	}

	if(vrsRt)
	{
		m_rts[kMaxColorRenderTargets + 1].m_handle = *vrsRt;
		m_vrsRtTexelSizeX = vrsRtTexelSizeX;
		m_vrsRtTexelSizeY = vrsRtTexelSizeY;
	}

	m_rpassRenderArea = {minx, miny, width, height};
}

inline RenderGraphDescription::~RenderGraphDescription()
{
	for(RenderPassDescriptionBase* pass : m_passes)
	{
		deleteInstance(*m_pool, pass);
	}
}

inline GraphicsRenderPassDescription& RenderGraphDescription::newGraphicsRenderPass(CString name)
{
	GraphicsRenderPassDescription* pass = newInstance<GraphicsRenderPassDescription>(*m_pool, this, m_pool);
	pass->setName(name);
	m_passes.emplaceBack(pass);
	return *pass;
}

inline ComputeRenderPassDescription& RenderGraphDescription::newComputeRenderPass(CString name)
{
	ComputeRenderPassDescription* pass = newInstance<ComputeRenderPassDescription>(*m_pool, this, m_pool);
	pass->setName(name);
	m_passes.emplaceBack(pass);
	return *pass;
}

inline RenderTargetHandle RenderGraphDescription::importRenderTarget(Texture* tex, TextureUsageBit usage)
{
	for([[maybe_unused]] const RT& rt : m_renderTargets)
	{
		ANKI_ASSERT(rt.m_importedTex.tryGet() != tex && "Already imported");
	}

	RT& rt = *m_renderTargets.emplaceBack();
	rt.m_importedTex.reset(tex);
	rt.m_importedLastKnownUsage = usage;
	rt.m_usageDerivedByDeps = TextureUsageBit::kNone;
	rt.setName(tex->getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

inline RenderTargetHandle RenderGraphDescription::importRenderTarget(Texture* tex)
{
	RenderTargetHandle out = importRenderTarget(tex, TextureUsageBit::kNone);
	m_renderTargets.getBack().m_importedAndUndefinedUsage = true;
	return out;
}

inline RenderTargetHandle RenderGraphDescription::newRenderTarget(const RenderTargetDescription& initInf)
{
	ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDescription::bake");
	ANKI_ASSERT(initInf.m_usage == TextureUsageBit::kNone && "Don't need to supply the usage. Render grap will find it");

	for([[maybe_unused]] auto it : m_renderTargets)
	{
		ANKI_ASSERT(it.m_hash != initInf.m_hash && "There is another RT descriptor with the same hash. Rendergraph's RT recycler will get confused");
	}

	RT& rt = *m_renderTargets.emplaceBack();
	rt.m_initInfo = initInf;
	rt.m_hash = initInf.m_hash;
	rt.m_importedLastKnownUsage = TextureUsageBit::kNone;
	rt.m_usageDerivedByDeps = TextureUsageBit::kNone;
	rt.setName(initInf.getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

inline BufferHandle RenderGraphDescription::importBuffer(Buffer* buff, BufferUsageBit usage, PtrSize offset, PtrSize range)
{
	// Checks
	ANKI_ASSERT(buff);
	if(range == kMaxPtrSize)
	{
		ANKI_ASSERT(offset < buff->getSize());
	}
	else
	{
		ANKI_ASSERT((offset + range) <= buff->getSize());
	}

	ANKI_ASSERT(range > 0);

	for([[maybe_unused]] const BufferRsrc& bb : m_buffers)
	{
		ANKI_ASSERT((bb.m_importedBuff.get() != buff || !bufferRangeOverlaps(bb.m_offset, bb.m_range, offset, range)) && "Range already imported");
	}

	BufferRsrc& b = *m_buffers.emplaceBack();
	b.setName(buff->getName());
	b.m_usage = usage;
	b.m_importedBuff.reset(buff);
	b.m_offset = offset;
	b.m_range = range;

	BufferHandle out;
	out.m_idx = m_buffers.getSize() - 1;
	return out;
}

inline AccelerationStructureHandle RenderGraphDescription::importAccelerationStructure(AccelerationStructure* as, AccelerationStructureUsageBit usage)
{
	for([[maybe_unused]] const AS& a : m_as)
	{
		ANKI_ASSERT(a.m_importedAs.get() != as && "Already imported");
	}

	AS& a = *m_as.emplaceBack();
	a.setName(as->getName());
	a.m_importedAs.reset(as);
	a.m_usage = usage;

	AccelerationStructureHandle handle;
	handle.m_idx = m_as.getSize() - 1;
	return handle;
}

} // end namespace anki
