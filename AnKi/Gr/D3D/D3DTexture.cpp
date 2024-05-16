// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>

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
	TextureGarbage* garbage = anki::newInstance<TextureGarbage>(GrMemoryPool::getSingleton());

	for(auto it : m_viewsMap)
	{
		garbage->m_descriptorHeapHandles.emplaceBack(it.m_handle);
		garbage->m_descriptorHeapHandleTypes.emplaceBack(it.m_heapType);
	}

	if(m_wholeTextureSrv.m_handle.isCreated())
	{
		garbage->m_descriptorHeapHandles.emplaceBack(m_wholeTextureSrv.m_handle);
		garbage->m_descriptorHeapHandleTypes.emplaceBack(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	if(m_firstSurfaceRtvOrDsv.m_handle.isCreated())
	{
		garbage->m_descriptorHeapHandles.emplaceBack(m_firstSurfaceRtvOrDsv.m_handle);
		garbage->m_descriptorHeapHandleTypes.emplaceBack((!!m_aspect) ? D3D12_DESCRIPTOR_HEAP_TYPE_DSV : D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	if(!isExternal())
	{
		garbage->m_resource = m_resource;
	}

	FrameGarbageCollector::getSingleton().newTextureGarbage(garbage);
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

	// Create the default views
	if(!!(m_usage & TextureUsageBit::kAllFramebuffer))
	{
		const TextureView tview(this, TextureSubresourceDescriptor::firstSurface());

		if(m_aspect == DepthStencilAspectBit::kNone)
		{
			m_firstSurfaceRtvOrDsv.m_heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		}
		else
		{
			m_firstSurfaceRtvOrDsv.m_heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			m_firstSurfaceRtvOrDsv.m_dsvReadOnly = false;
		}

		m_firstSurfaceRtvOrDsv.m_handle =
			createDescriptorHeapHandle(tview.getSubresource(), m_firstSurfaceRtvOrDsv.m_heapType, m_firstSurfaceRtvOrDsv.m_dsvReadOnly);
		m_firstSurfaceRtvOrDsvSubresource = tview.getSubresource();
	}

	if(!!(m_usage & TextureUsageBit::kAllSampled))
	{
		const TextureView tview(this, TextureSubresourceDescriptor::all());

		m_wholeTextureSrv.m_heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_wholeTextureSrv.m_handle = createDescriptorHeapHandle(tview.getSubresource(), m_firstSurfaceRtvOrDsv.m_heapType, false);

		m_wholeTextureSrvSubresource = tview.getSubresource();
	}

	return Error::kNone;
}

const TextureImpl::View& TextureImpl::getOrCreateView(const TextureSubresourceDescriptor& subresource, D3D12_DESCRIPTOR_HEAP_TYPE heapType,
													  TextureUsageBit usage) const
{
	ANKI_ASSERT(subresource == TextureView(this, subresource).getSubresource() && "Should have been sanitized");

	const Bool readOnlyDsv =
		m_aspect != DepthStencilAspectBit::kNone && (usage & TextureUsageBit::kAllFramebuffer) == TextureUsageBit::kFramebufferRead;

	if(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && subresource == m_wholeTextureSrvSubresource)
	{
		return m_wholeTextureSrv;
	}
	else if((heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
			&& subresource == m_firstSurfaceRtvOrDsvSubresource && m_firstSurfaceRtvOrDsv.m_dsvReadOnly == readOnlyDsv)
	{
		return m_firstSurfaceRtvOrDsv;
	}

	// Slow path

	ANKI_BEGIN_PACKED_STRUCT
	class
	{
	public:
		TextureSubresourceDescriptor m_subresource;
		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		Bool m_readOnlyDsv;
	} toHash = {subresource, heapType, readOnlyDsv};
	ANKI_END_PACKED_STRUCT

	const U64 hash = computeHash(&toHash, sizeof(toHash));

	View* view = nullptr;
	{
		RLockGuard lock(m_viewsMapMtx);

		auto it = m_viewsMap.find(hash);
		if(it != m_viewsMap.getEnd())
		{
			view = &(*it);
		}
	}

	if(view)
	{
		// Found, return it
		return *view;
	}

	WLockGuard lock(m_viewsMapMtx);

	// Search again
	auto it = m_viewsMap.find(hash);
	if(it != m_viewsMap.getEnd())
	{
		return *it;
	}

	// Need to create it
	View& nview = *m_viewsMap.emplace(hash, *view);
	nview.m_heapType = heapType;
	nview.m_handle = createDescriptorHeapHandle(subresource, heapType, readOnlyDsv);
	nview.m_dsvReadOnly = readOnlyDsv;

	return nview;
}

DescriptorHeapHandle TextureImpl::createDescriptorHeapHandle(const TextureSubresourceDescriptor& subresource, D3D12_DESCRIPTOR_HEAP_TYPE heapType,
															 Bool readOnlyDsv) const
{
	DescriptorHeapHandle handle;

	if(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	{
		ANKI_ASSERT(TextureView(this, subresource).isGoodForRenderTarget() && m_aspect == DepthStencilAspectBit::kNone);

		D3D12_RENDER_TARGET_VIEW_DESC desc = {};

		if(m_texType == TextureType::k2D)
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = subresource.m_mipmap;
			desc.Texture2D.PlaneSlice = 0;
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.ArraySize = 1;
			desc.Texture2DArray.FirstArraySlice = subresource.m_layer * m_layerCount + subresource.m_face;
			desc.Texture2DArray.MipSlice = subresource.m_mipmap;
		}

		handle = DescriptorFactory::getSingleton().allocateCpuVisiblePersistent(heapType);
		getDevice().CreateRenderTargetView(m_resource, (isExternal()) ? nullptr : &desc, handle.m_cpuHandle);
	}
	else if(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
	{
		ANKI_ASSERT(TextureView(this, subresource).isGoodForRenderTarget() && m_aspect != DepthStencilAspectBit::kNone);

		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT_UNKNOWN; // TODO

		if(readOnlyDsv)
		{
			desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
			desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		}

		if(m_texType == TextureType::k2D)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = subresource.m_mipmap;
		}
		else
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.ArraySize = 1;
			desc.Texture2DArray.FirstArraySlice = subresource.m_layer * m_layerCount + subresource.m_face;
			desc.Texture2DArray.MipSlice = subresource.m_mipmap;
		}

		handle = DescriptorFactory::getSingleton().allocateCpuVisiblePersistent(heapType);
		getDevice().CreateDepthStencilView(m_resource, &desc, handle.m_cpuHandle);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	return handle;
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
