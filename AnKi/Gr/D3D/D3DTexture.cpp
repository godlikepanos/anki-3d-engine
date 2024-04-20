// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DTexture.h>

namespace anki {

Texture* Texture::newInstance(const TextureInitInfo& init)
{
	TextureImpl* impl = anki::newInstance<TextureImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

TextureImpl::~TextureImpl()
{
	for(DescriptorHeapHandle& handle : m_rtvHandles)
	{
		RtvDescriptorHeap::getSingleton().free(handle);
	}

	const Bool external = !!(m_usage & TextureUsageBit::kPresent);
	if(!external)
	{
		safeRelease(m_resource);
	}
}

Error TextureImpl::initInternal(ID3D12Resource* external, const TextureInitInfo& init)
{
	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_layerCount = init.m_layerCount;
	m_texType = init.m_type;
	m_usage = init.m_usage;
	m_format = init.m_format;
	m_aspect = getFormatInfo(init.m_format).isDepth() ? DepthStencilAspectBit::kDepth : DepthStencilAspectBit::kNone;
	m_aspect |= getFormatInfo(init.m_format).isStencil() ? DepthStencilAspectBit::kStencil : DepthStencilAspectBit::kNone;

	if(m_texType == TextureType::k3D)
	{
		m_mipCount = min<U32>(init.m_mipmapCount, computeMaxMipmapCount3d(m_width, m_height, m_depth));
	}
	else
	{
		m_mipCount = min<U32>(init.m_mipmapCount, computeMaxMipmapCount2d(m_width, m_height));
	}

	if(external)
	{
		ANKI_ASSERT(!!(m_usage & TextureUsageBit::kPresent));
		m_resource = external;
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	const U32 faceCount = (m_texType == TextureType::kCube || m_texType == TextureType::kCubeArray) ? 6 : 1;
	U32 surfaceCount = 0;
	if(m_texType != TextureType::k3D)
	{
		surfaceCount = m_layerCount * m_mipCount * faceCount;
	}

	// Create RTVs
	if(!!(m_usage & TextureUsageBit::kAllFramebuffer))
	{
		ANKI_ASSERT(m_texType != TextureType::k3D && m_texType != TextureType::k1D);
		m_rtvHandles.resize(surfaceCount);
		for(U32 layer = 0; layer < m_layerCount; ++layer)
		{
			for(U32 face = 0; face < faceCount; ++face)
			{
				for(U32 mip = 0; mip < m_mipCount; ++mip)
				{
					D3D12_RENDER_TARGET_VIEW_DESC desc = {};

					if(m_texType == TextureType::k2D)
					{
						desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						desc.Texture2D.MipSlice = mip;
						desc.Texture2D.PlaneSlice = 0;
					}
					else
					{
						desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						desc.Texture2DArray.ArraySize = 1;
						desc.Texture2DArray.FirstArraySlice = layer * m_layerCount + face;
					}

					DescriptorHeapHandle& handle = m_rtvHandles[layer * faceCount * m_mipCount + face * m_mipCount + mip];
					handle = RtvDescriptorHeap::getSingleton().allocate();
					getDevice().CreateRenderTargetView(m_resource, (external) ? nullptr : &desc, handle.m_cpuHandle);
				}
			}
		}
	}

	return Error::kNone;
}

} // end namespace anki
