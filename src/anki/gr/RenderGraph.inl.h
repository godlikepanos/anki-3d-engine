// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/RenderGraph.h>

namespace anki
{

inline void RenderPassWorkContext::getBufferState(RenderPassBufferHandle handle, BufferPtr& buff) const
{
	buff = m_rgraph->getBuffer(handle);
}

inline void RenderPassWorkContext::getRenderTargetState(
	RenderTargetHandle handle, const TextureSubresourceInfo& subresource, TexturePtr& tex, TextureUsageBit& usage) const
{
	m_rgraph->getCrntUsage(handle, m_passIdx, subresource, usage);
	tex = m_rgraph->getTexture(handle);
}

inline TexturePtr RenderPassWorkContext::getTexture(RenderTargetHandle handle) const
{
	return m_rgraph->getTexture(handle);
}

inline void RenderPassDescriptionBase::fixSubresource(RenderPassDependency& dep) const
{
	ANKI_ASSERT(dep.m_isTexture);

	TextureSubresourceInfo& subresource = dep.m_texture.m_subresource;
	const Bool wholeTexture = subresource.m_mipmapCount == MAX_U32;
	if(wholeTexture)
	{
		ANKI_ASSERT(subresource.m_firstFace == 0);
		ANKI_ASSERT(subresource.m_firstMipmap == 0);
		ANKI_ASSERT(subresource.m_firstLayer == 0);

		const RenderGraphDescription::RT& rt = m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx];
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
}

inline void RenderPassDescriptionBase::newConsumer(const RenderPassDependency& dep)
{
	m_consumers.emplaceBack(m_alloc, dep);

	if(dep.m_isTexture && dep.m_texture.m_usage != TextureUsageBit::NONE)
	{
		fixSubresource(m_consumers.getBack());
		m_consumerRtMask.set(dep.m_texture.m_handle.m_idx);

		// Try to derive the usage by that dep
		m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx].m_usageDerivedByDeps |= dep.m_texture.m_usage;
	}
	else if(dep.m_buffer.m_usage != BufferUsageBit::NONE)
	{
		m_consumerBufferMask.set(dep.m_buffer.m_handle.m_idx);
		m_hasBufferDeps = true;
	}
}

inline void RenderPassDescriptionBase::newProducer(const RenderPassDependency& dep)
{
	m_producers.emplaceBack(m_alloc, dep);
	if(dep.m_isTexture)
	{
		fixSubresource(m_producers.getBack());
		m_producerRtMask.set(dep.m_texture.m_handle.m_idx);
	}
	else
	{
		m_producerBufferMask.set(dep.m_buffer.m_handle.m_idx);
		m_hasBufferDeps = true;
	}
}

} // end namespace anki