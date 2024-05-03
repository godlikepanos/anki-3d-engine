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

U32 Texture::getOrCreateBindlessTextureIndex(const TextureSubresourceDescriptor& subresource)
{
	ANKI_ASSERT(!"TODO");
	return 0;
}

TextureImpl::~TextureImpl()
{
	auto deleteView = [](TextureViewEntry& entry, D3DTextureViewType type) {
		DescriptorHeap* heap = nullptr;
		switch(type)
		{
		case D3DTextureViewType::kSrv:
		case D3DTextureViewType::kUav:
			heap = &CbvSrvUavDescriptorHeap::getSingleton();
			break;
		case D3DTextureViewType::kRtv:
			heap = &RtvDescriptorHeap::getSingleton();
			break;
		case D3DTextureViewType::kDsv:
			heap = nullptr;
			break;
		default:
			ANKI_ASSERT(0);
		}

		heap->free(entry.m_handle);
	};

	for(D3DTextureViewType c : EnumIterable<D3DTextureViewType>())
	{
		for(TextureViewEntry& entry : m_singleSurfaceOrVolumeViews[c])
		{
			deleteView(entry, c);
		}

		deleteView(m_wholeTextureViews[c], c);
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

	const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
	U32 surfaceCount = 0;
	if(m_texType != TextureType::k3D)
	{
		surfaceCount = m_layerCount * m_mipCount * faceCount;
	}

	// Create RTVs
	if(!!(m_usage & TextureUsageBit::kAllFramebuffer))
	{
		ANKI_ASSERT(m_texType != TextureType::k3D && m_texType != TextureType::k1D);
		ANKI_ASSERT(!getFormatInfo(m_format).isDepthStencil() && "TODO");

		m_singleSurfaceOrVolumeViews[D3DTextureViewType::kRtv].resize(surfaceCount);

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

					DescriptorHeapHandle& handle =
						m_singleSurfaceOrVolumeViews[D3DTextureViewType::kRtv][layer * faceCount * m_mipCount + face * m_mipCount + mip].m_handle;
					handle = RtvDescriptorHeap::getSingleton().allocate();
					getDevice().CreateRenderTargetView(m_resource, (external) ? nullptr : &desc, handle.m_cpuHandle);
				}
			}
		}
	}

	return Error::kNone;
}

const TextureImpl::TextureViewEntry& TextureImpl::getViewEntry(const TextureSubresourceDescriptor& subresource, D3DTextureViewType viewType) const
{
	const TextureView view(this, subresource);
	const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;

	const TextureViewEntry* entry;
	if(viewType == D3DTextureViewType::kRtv)
	{
		ANKI_ASSERT(view.isGoodForRenderTarget());
		const U32 idx = subresource.m_layer * faceCount * m_mipCount + subresource.m_face * m_mipCount + subresource.m_mipmap;
		entry = &m_singleSurfaceOrVolumeViews[D3DTextureViewType::kRtv][idx];
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	ANKI_ASSERT(entry);
	return *entry;
}

void TextureImpl::computeResourceStates(TextureUsageBit usage, D3D12_RESOURCE_STATES& states) const
{
	ANKI_ASSERT((usage & m_usage) == usage);

	if(usage == TextureUsageBit::kNone)
	{
		// D3D doesn't have a clean slade, figure something out

		states |= D3D12_RESOURCE_STATE_COMMON;
	}
	else
	{
		states = D3D12_RESOURCE_STATES(0);
		if(!!(usage & TextureUsageBit::kAllFramebuffer))
		{
			states |= D3D12_RESOURCE_STATE_RENDER_TARGET;

			if(!!(usage & TextureUsageBit::kFramebufferWrite) && !!(m_aspect & DepthStencilAspectBit::kDepth))
			{
				states |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}

			if(!!(usage & TextureUsageBit::kFramebufferRead) && !!(m_aspect & DepthStencilAspectBit::kDepth))
			{
				states |= D3D12_RESOURCE_STATE_DEPTH_READ;
			}
		}

		if(!!(usage & TextureUsageBit::kAllStorage))
		{
			states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}

		if(!!(usage & TextureUsageBit::kSampledFragment))
		{
			states |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}

		if(!!(usage & (TextureUsageBit::kAllSampled & ~TextureUsageBit::kSampledFragment)))
		{
			states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}

		if(!!(usage & TextureUsageBit::kTransferDestination))
		{
			states |= D3D12_RESOURCE_STATE_COPY_DEST;
		}

		if(!!(usage & TextureUsageBit::kFramebufferShadingRate))
		{
			states |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
		}

		if(!!(usage & TextureUsageBit::kPresent))
		{
			states |= D3D12_RESOURCE_STATE_PRESENT;
		}
	}
}

void TextureImpl::computeBarrierInfo(TextureUsageBit before, TextureUsageBit after, D3D12_RESOURCE_STATES& statesBefore,
									 D3D12_RESOURCE_STATES& statesAfter) const
{
	computeResourceStates(before, statesBefore);
	computeResourceStates(after, statesAfter);
}

} // end namespace anki
