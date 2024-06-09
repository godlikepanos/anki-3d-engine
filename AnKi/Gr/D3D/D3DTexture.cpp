// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

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
		desc.Flags = {};

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

		ANKI_ASSERT(tview.getSubresource().m_depthStencilAspect != DepthStencilAspectBit::kDepthStencil && "Can only create a single plane SRV");

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

		if(!m_aspect)
		{
			desc.Format = DXGI_FORMAT(m_format);
		}
		else
		{
			// D3D doesn't like depth formats as SRVs
			switch(m_format)
			{
			case Format::kD16_Unorm:
				desc.Format = DXGI_FORMAT(Format::kR16_Unorm);
				break;
			case Format::kD32_Sfloat:
				desc.Format = DXGI_FORMAT(Format::kR32_Sfloat);
				break;
			case Format::kD24_Unorm_S8_Uint:
				if(tview.getSubresource().m_depthStencilAspect == DepthStencilAspectBit::kDepth)
				{
					desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				}
				else
				{
					desc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
				}
				break;
			case Format::kD32_Sfloat_S8_Uint:
				if(tview.getSubresource().m_depthStencilAspect == DepthStencilAspectBit::kDepth)
				{
					desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				}
				else
				{
					desc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
				}
				break;
			default:
				ANKI_ASSERT(0);
			}
		}

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

D3D12_TEXTURE_BARRIER TextureImpl::computeBarrierInfo(TextureUsageBit before, TextureUsageBit after,
													  const TextureSubresourceDescriptor& subresource) const
{
	ANKI_ASSERT((m_usage & before) == before);
	ANKI_ASSERT((m_usage & after) == after);
	ANKI_ASSERT(subresource == TextureView(this, subresource).getSubresource() && "Should have been sanitized");

	D3D12_TEXTURE_BARRIER barrier;

	computeBarrierInfo(before, barrier.SyncBefore, barrier.AccessBefore);
	computeBarrierInfo(after, barrier.SyncAfter, barrier.AccessAfter);
	barrier.LayoutBefore = computeLayout(before);
	barrier.LayoutAfter = computeLayout(after);
	barrier.pResource = m_resource;

	barrier.Subresources = {};
	if(subresource.m_allSurfacesOrVolumes)
	{
		barrier.Subresources.IndexOrFirstMipLevel = kMaxU32;
	}
	else
	{
		const U8 faceCount = textureTypeIsCube(m_texType) ? 6 : 1;

		barrier.Subresources.IndexOrFirstMipLevel = subresource.m_mipmap;
		barrier.Subresources.NumMipLevels = 1;
		barrier.Subresources.FirstArraySlice = subresource.m_layer * faceCount + subresource.m_face;
		barrier.Subresources.NumArraySlices = 1;

		if(!!(subresource.m_depthStencilAspect & DepthStencilAspectBit::kDepth) || subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone)
		{
			barrier.Subresources.FirstPlane = 0;
		}
		else
		{
			barrier.Subresources.FirstPlane = 1;
		}

		barrier.Subresources.NumPlanes = (subresource.m_depthStencilAspect == DepthStencilAspectBit::kDepthStencil) ? 2 : 1;
	}

	if(before == TextureUsageBit::kNone)
	{
		barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_DISCARD;
	}

	return barrier;
}

void TextureImpl::computeBarrierInfo(TextureUsageBit usage, D3D12_BARRIER_SYNC& stages, D3D12_BARRIER_ACCESS& accesses) const
{
	if(usage == TextureUsageBit::kNone)
	{
		stages = D3D12_BARRIER_SYNC_NONE;
		accesses = D3D12_BARRIER_ACCESS_NO_ACCESS;
		return;
	}

	stages = {};
	accesses = {};
	const Bool depthStencil = !!m_aspect;
	const Bool rt = getGrManagerImpl().getDeviceCapabilities().m_rayTracingEnabled;

	if(!!(usage & (TextureUsageBit::kSampledGeometry | TextureUsageBit::kStorageGeometryRead)))
	{
		stages |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	}

	if(!!(usage & TextureUsageBit::kStorageGeometryWrite))
	{
		stages |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	}

	if(!!(usage & (TextureUsageBit::kSampledFragment | TextureUsageBit::kStorageFragmentRead)))
	{
		stages |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	}

	if(!!(usage & TextureUsageBit::kStorageFragmentWrite))
	{
		stages |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	}

	if(!!(usage & (TextureUsageBit::kSampledCompute | TextureUsageBit::kStorageComputeRead)))
	{
		stages |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	}

	if(!!(usage & TextureUsageBit::kStorageComputeWrite))
	{
		stages |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	}

	if(!!(usage & (TextureUsageBit::kSampledTraceRays | TextureUsageBit::kStorageTraceRaysRead)) && rt)
	{
		stages |= D3D12_BARRIER_SYNC_RAYTRACING;
		accesses |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
	}

	if(!!(usage & TextureUsageBit::kStorageTraceRaysWrite) && rt)
	{
		stages |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
	}

	if(!!(usage & TextureUsageBit::kFramebufferRead))
	{
		if(depthStencil)
		{
			stages |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			accesses |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
		}
		else
		{
			stages |= D3D12_BARRIER_SYNC_RENDER_TARGET;
			accesses |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
		}
	}

	if(!!(usage & TextureUsageBit::kFramebufferWrite))
	{
		if(depthStencil)
		{
			stages |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			accesses |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
		}
		else
		{
			stages |= D3D12_BARRIER_SYNC_RENDER_TARGET;
			accesses |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
		}
	}

	if(!!(usage & TextureUsageBit::kFramebufferShadingRate))
	{
		stages |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
		accesses |= D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;
	}

	if(!!(usage & TextureUsageBit::kGenerateMipmaps))
	{
		ANKI_ASSERT(!"TODO rm");
	}

	if(!!(usage & TextureUsageBit::kTransferDestination))
	{
		stages |= D3D12_BARRIER_SYNC_COPY;
		accesses |= D3D12_BARRIER_ACCESS_COPY_DEST;
	}

	if(!!(usage & TextureUsageBit::kPresent))
	{
		stages |= D3D12_BARRIER_SYNC_COPY;
		accesses |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
	}
}

D3D12_BARRIER_LAYOUT TextureImpl::computeLayout(TextureUsageBit usage) const
{
	const Bool depthStencil = !!m_aspect;
	D3D12_BARRIER_LAYOUT out = {};

	if(usage == TextureUsageBit::kNone)
	{
		out = D3D12_BARRIER_LAYOUT_UNDEFINED;
	}
	else if(depthStencil)
	{
		if((usage & TextureUsageBit::kAllSampled) == usage)
		{
			// Only sampled
			out = D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
		}
		else if((usage & TextureUsageBit::kFramebufferRead) == usage)
		{
			// Only FB read
			out = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
		}
		else if((usage & (TextureUsageBit::kAllSampled | TextureUsageBit::kFramebufferRead)) == usage)
		{
			// Only depth tests and sampled
			out = D3D12_BARRIER_LAYOUT_COMMON;
		}
		else
		{
			// Only attachment write, the rest (eg transfer) are not supported for now
			ANKI_ASSERT(usage == TextureUsageBit::kAllFramebuffer || usage == TextureUsageBit::kFramebufferWrite);
			out = D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
		}
	}
	else if((usage & TextureUsageBit::kAllFramebuffer) == usage)
	{
		// Color attachment
		out = D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	}
	else if((usage & TextureUsageBit::kFramebufferShadingRate) == usage)
	{
		// SRI
		out = D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
	}
	else if((usage & TextureUsageBit::kAllUav) == usage)
	{
		// UAV
		out = D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
	}
	else if((usage & TextureUsageBit::kAllSrv) == usage)
	{
		// SRV
		out = D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
	}
	else if(usage == TextureUsageBit::kGenerateMipmaps)
	{
		ANKI_ASSERT(!"TODO rm");
	}
	else if(usage == TextureUsageBit::kTransferDestination)
	{
		out = D3D12_BARRIER_LAYOUT_COPY_DEST;
	}
	else if(usage == TextureUsageBit::kPresent)
	{
		out = D3D12_BARRIER_LAYOUT_PRESENT;
	}
	else
	{
		// No idea so play it safe
		out = D3D12_BARRIER_LAYOUT_COMMON;
	}

	return out;
}

} // end namespace anki
