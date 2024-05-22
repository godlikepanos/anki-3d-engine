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
	ANKI_D3D_SELF(TextureImpl);

	const TextureImpl::View& view = self.getOrCreateView(subresource, TextureUsageBit::kAllSampled);

	LockGuard lock(view.m_bindlessLock);

	if(view.m_bindlessIndex == kMaxU32)
	{
		view.m_bindlessHandle = DescriptorFactory::getSingleton().allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
		view.m_bindlessIndex = DescriptorFactory::getSingleton().getBindlessIndex(view.m_bindlessHandle);
	}

	return view.m_bindlessIndex;
}

TextureImpl::~TextureImpl()
{
	TextureGarbage* garbage = anki::newInstance<TextureGarbage>(GrMemoryPool::getSingleton());

	for(auto& it : m_viewsMap)
	{
		garbage->m_descriptorHeapHandles.emplaceBack(it.m_handle);
		if(it.m_bindlessHandle.isCreated())
		{
			garbage->m_descriptorHeapHandles.emplaceBack(it.m_bindlessHandle);
		}
	}

	if(m_wholeTextureSrv.m_handle.isCreated())
	{
		garbage->m_descriptorHeapHandles.emplaceBack(m_wholeTextureSrv.m_handle);
		if(m_wholeTextureSrv.m_bindlessHandle.isCreated())
		{
			garbage->m_descriptorHeapHandles.emplaceBack(m_wholeTextureSrv.m_bindlessHandle);
		}
	}

	if(m_firstSurfaceRtvOrDsv.m_handle.isCreated())
	{
		garbage->m_descriptorHeapHandles.emplaceBack(m_firstSurfaceRtvOrDsv.m_handle);
	}

	if(!isExternal())
	{
		garbage->m_resource = m_resource;
	}

	FrameGarbageCollector::getSingleton().newTextureGarbage(garbage);
}

Error TextureImpl::initInternal(ID3D12Resource* external, const TextureInitInfo& init)
{
	ANKI_ASSERT(init.isValid());
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
		ANKI_ASSERT(!(m_usage & TextureUsageBit::kPresent));

		const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;

		D3D12_RESOURCE_DESC desc = {};
		if(m_texType == TextureType::k1D)
		{
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			desc.DepthOrArraySize = U16(init.m_layerCount);
		}
		else if(m_texType == TextureType::k3D)
		{
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			desc.DepthOrArraySize = U16(init.m_depth);
		}
		else
		{
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.DepthOrArraySize = U16(init.m_layerCount * faceCount);
		}
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = U16(m_mipCount);
		desc.Format = DXGI_FORMAT(m_format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

		if(!!(m_usage & TextureUsageBit::kAllFramebuffer) && m_aspect == DepthStencilAspectBit::kNone)
		{
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}

		if(!!(m_usage & TextureUsageBit::kAllFramebuffer) && m_aspect != DepthStencilAspectBit::kNone)
		{
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}

		if(!!(m_usage & TextureUsageBit::kAllStorage))
		{
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		if(!(m_usage & TextureUsageBit::kAllShaderResource))
		{
			desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_HEAP_FLAGS heapFlags = {};
		if(!!(m_usage & TextureUsageBit::kAllStorage))
		{
			heapFlags |= D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS;
		}

		const D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
		ANKI_D3D_CHECK(getDevice().CreateCommittedResource(&heapProperties, heapFlags, &desc, initialState, nullptr, IID_PPV_ARGS(&m_resource)));

		ANKI_D3D_CHECK(m_resource->SetName(s2ws(init.getName().cstr()).c_str()));
	}

	// Create the default views
	if(!!(m_usage & TextureUsageBit::kAllFramebuffer))
	{
		const TextureView tview(this, TextureSubresourceDescriptor::firstSurface());
		initView(tview.getSubresource(), TextureUsageBit::kAllFramebuffer, m_firstSurfaceRtvOrDsv);
		m_firstSurfaceRtvOrDsvSubresource = tview.getSubresource();
	}

	if(!!(m_usage & TextureUsageBit::kAllSampled))
	{
		const TextureView tview(this, TextureSubresourceDescriptor::all());
		initView(tview.getSubresource(), TextureUsageBit::kAllSampled, m_wholeTextureSrv);
		m_wholeTextureSrvSubresource = tview.getSubresource();
	}

	return Error::kNone;
}

void TextureImpl::initView(const TextureSubresourceDescriptor& subresource, TextureUsageBit usage, View& view) const
{
	ANKI_ASSERT(!!(m_usage & usage));

	const Bool rtv = !!(usage & TextureUsageBit::kAllFramebuffer) && m_aspect == DepthStencilAspectBit::kNone;
	const Bool dsv = !!(usage & TextureUsageBit::kAllFramebuffer) && m_aspect != DepthStencilAspectBit::kNone;
	const Bool srv = !!(usage & TextureUsageBit::kAllSampled);
	const Bool uav = !!(usage & TextureUsageBit::kAllStorage);

	ANKI_ASSERT(rtv + dsv + srv + uav == 1 && "Only enable one");

	view.m_usage = usage;

	if(rtv)
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

		view.m_handle = DescriptorFactory::getSingleton().allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
		getDevice().CreateRenderTargetView(m_resource, (isExternal()) ? nullptr : &desc, view.m_handle.getCpuOffset());
	}
	else if(dsv)
	{
		ANKI_ASSERT(TextureView(this, subresource).isGoodForRenderTarget() && m_aspect != DepthStencilAspectBit::kNone);

		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT(m_format);

		const Bool readOnlyDsv = !(usage & TextureUsageBit::kFramebufferWrite);
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

		view.m_handle = DescriptorFactory::getSingleton().allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);
		getDevice().CreateDepthStencilView(m_resource, &desc, view.m_handle.getCpuOffset());
	}
	else if(srv)
	{
		const TextureView tview(this, subresource);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT(m_format);

		const U32 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;
		const U32 surfaceCount = faceCount * m_layerCount;
		const Bool reallySingleSurfaceOrVol = !subresource.m_allSurfacesOrVolumes || surfaceCount == 1;
		const U32 plane = !!(subresource.m_depthStencilAspect & DepthStencilAspectBit::kStencil) ? 1 : 0;

		if(m_texType == TextureType::k1D)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MostDetailedMip = tview.getFirstMipmap();
			desc.Texture1D.MipLevels = tview.getMipmapCount();
			desc.Texture1D.ResourceMinLODClamp = 0.0f;
		}
		else if(m_texType == TextureType::k2D)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MostDetailedMip = tview.getFirstMipmap();
			desc.Texture2D.MipLevels = tview.getMipmapCount();
			desc.Texture2D.PlaneSlice = plane;
			desc.Texture2D.ResourceMinLODClamp = 0.0f;
		}
		else if(m_texType == TextureType::k2DArray)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MostDetailedMip = tview.getFirstMipmap();
			desc.Texture2DArray.MipLevels = tview.getMipmapCount();
			desc.Texture2DArray.FirstArraySlice = tview.getFirstLayer();
			desc.Texture2DArray.ArraySize = tview.getLayerCount();
			desc.Texture2DArray.PlaneSlice = plane;
			desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		}
		else if(m_texType == TextureType::k3D)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MostDetailedMip = tview.getFirstMipmap();
			desc.Texture3D.MipLevels = tview.getMipmapCount();
			desc.Texture3D.ResourceMinLODClamp = 0.0f;
		}
		else if(m_texType == TextureType::kCube)
		{
			if(reallySingleSurfaceOrVol)
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = tview.getFirstMipmap();
				desc.Texture2DArray.MipLevels = tview.getMipmapCount();
				desc.Texture2DArray.FirstArraySlice = tview.getFirstFace();
				desc.Texture2DArray.ArraySize = 1;
				desc.Texture2DArray.PlaneSlice = plane;
				desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				desc.TextureCube.MostDetailedMip = tview.getFirstMipmap();
				desc.TextureCube.MipLevels = tview.getMipmapCount();
				desc.TextureCube.ResourceMinLODClamp = 0.0f;
			}
		}
		else if(m_texType == TextureType::kCubeArray)
		{
			if(reallySingleSurfaceOrVol)
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = tview.getFirstMipmap();
				desc.Texture2DArray.MipLevels = tview.getMipmapCount();
				desc.Texture2DArray.FirstArraySlice = tview.getFirstLayer() * 6 + tview.getFirstFace();
				desc.Texture2DArray.ArraySize = 1;
				desc.Texture2DArray.PlaneSlice = plane;
				desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				desc.TextureCubeArray.MostDetailedMip = tview.getFirstMipmap();
				desc.TextureCubeArray.MipLevels = tview.getMipmapCount();
				desc.TextureCubeArray.First2DArrayFace = 0;
				desc.TextureCubeArray.NumCubes = tview.getLayerCount();
				desc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
			}
		}
		else
		{
			ANKI_ASSERT(0);
		}

		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		view.m_handle = DescriptorFactory::getSingleton().allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false);
		getDevice().CreateShaderResourceView(m_resource, &desc, view.m_handle.getCpuOffset());
	}
	else if(uav)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT(m_format);

		if(m_texType == TextureType::k1D)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = subresource.m_mipmap;
		}
		else if(m_texType == TextureType::k2D)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = subresource.m_mipmap;
			desc.Texture2D.PlaneSlice = 0;
		}
		else if(m_texType == TextureType::k2DArray)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = subresource.m_mipmap;
			desc.Texture2DArray.FirstArraySlice = subresource.m_layer;
			desc.Texture2DArray.ArraySize = 1;
			desc.Texture2DArray.PlaneSlice = 0;
		}
		else if(m_texType == TextureType::k3D)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = subresource.m_mipmap;
			desc.Texture3D.FirstWSlice = 0;
			desc.Texture3D.WSize = U32(-1);
		}
		else if(m_texType == TextureType::kCube)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = subresource.m_mipmap;
			desc.Texture2DArray.FirstArraySlice = subresource.m_face;
			desc.Texture2DArray.ArraySize = 1;
			desc.Texture2DArray.PlaneSlice = 0;
		}
		else if(m_texType == TextureType::kCubeArray)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = subresource.m_mipmap;
			desc.Texture2DArray.FirstArraySlice = subresource.m_layer * 6 + subresource.m_face;
			desc.Texture2DArray.ArraySize = 1;
			desc.Texture2DArray.PlaneSlice = 0;
		}
		else
		{
			ANKI_ASSERT(0);
		}

		view.m_handle = DescriptorFactory::getSingleton().allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false);
		getDevice().CreateUnorderedAccessView(m_resource, nullptr, &desc, view.m_handle.getCpuOffset());
	}
	else
	{
		ANKI_ASSERT(0);
	}
}

const TextureImpl::View& TextureImpl::getOrCreateView(const TextureSubresourceDescriptor& subresource, TextureUsageBit usage) const
{
	ANKI_ASSERT(subresource == TextureView(this, subresource).getSubresource() && "Should have been sanitized");
	ANKI_ASSERT(!!(usage & m_usage));

	// Check some pre-created
	if(!!(usage & TextureUsageBit::kAllSampled) && subresource == m_wholeTextureSrvSubresource)
	{
		return m_wholeTextureSrv;
	}
	else if(usage == TextureUsageBit::kAllFramebuffer && subresource == m_firstSurfaceRtvOrDsvSubresource)
	{
		return m_firstSurfaceRtvOrDsv;
	}

	// Slow path

	ANKI_BEGIN_PACKED_STRUCT
	class
	{
	public:
		TextureSubresourceDescriptor m_subresource;
		TextureUsageBit m_usage;
	} toHash = {subresource, usage};
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
	View& nview = *m_viewsMap.emplace(hash);
	initView(subresource, usage, nview);

	return nview;
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
