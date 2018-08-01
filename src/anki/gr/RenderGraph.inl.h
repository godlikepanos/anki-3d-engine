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

inline void RenderPassDescriptionBase::validateDep(const RenderPassDependency& dep)
{
	// Validate dep
	if(dep.m_isTexture)
	{
		TextureUsageBit usage = dep.m_texture.m_usage;
		(void)usage;
		if(m_type == Type::GRAPHICS)
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::ALL_COMPUTE));
		}
		else
		{
			ANKI_ASSERT(!(usage & TextureUsageBit::ALL_GRAPHICS));
		}

		ANKI_ASSERT(!!(usage & TextureUsageBit::ALL_READ) || !!(usage & TextureUsageBit::ALL_WRITE));
	}
	else
	{
		BufferUsageBit usage = dep.m_buffer.m_usage;
		(void)usage;
		if(m_type == Type::GRAPHICS)
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::ALL_COMPUTE));
		}
		else
		{
			ANKI_ASSERT(!(usage & BufferUsageBit::ALL_GRAPHICS));
		}

		ANKI_ASSERT(!!(usage & BufferUsageBit::ALL_READ) || !!(usage & BufferUsageBit::ALL_WRITE));
	}
}

inline void RenderPassDescriptionBase::newDependency(const RenderPassDependency& dep)
{
	validateDep(dep);

	DynamicArray<RenderPassDependency>& deps = (dep.m_isTexture) ? m_rtDeps : m_buffDeps;
	deps.emplaceBack(m_alloc, dep);

	if(dep.m_isTexture)
	{
		fixSubresource(deps.getBack());

		if(!!(dep.m_texture.m_usage & TextureUsageBit::ALL_READ))
		{
			m_readRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		if(!!(dep.m_texture.m_usage & TextureUsageBit::ALL_WRITE))
		{
			m_writeRtMask.set(dep.m_texture.m_handle.m_idx);
		}

		// Try to derive the usage by that dep
		m_descr->m_renderTargets[dep.m_texture.m_handle.m_idx].m_usageDerivedByDeps |= dep.m_texture.m_usage;
	}
	else
	{
		if(!!(dep.m_buffer.m_usage & BufferUsageBit::ALL_READ))
		{
			m_readBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}

		if(!!(dep.m_buffer.m_usage & BufferUsageBit::ALL_WRITE))
		{
			m_writeBuffMask.set(dep.m_buffer.m_handle.m_idx);
		}

		m_hasBufferDeps = true;
	}
}

} // end namespace anki