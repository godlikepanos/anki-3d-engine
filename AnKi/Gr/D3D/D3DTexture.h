// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DDescriptorHeap.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Texture container.
class TextureImpl final : public Texture
{
	friend class Texture;

public:
	TextureImpl(CString name)
		: Texture(name)
	{
	}

	~TextureImpl();

	Error init(const TextureInitInfo& inf)
	{
		return initInternal(nullptr, inf);
	}

	Error initExternal(ID3D12Resource* external, const TextureInitInfo& init)
	{
		return initInternal(external, init);
	}

	DescriptorHeapHandle getOrCreateRtv(const TextureSubresourceDescriptor& subresource) const
	{
		const View& e = getOrCreateView(subresource, D3DTextureViewType::kRtv, TextureUsageBit::kNone);
		ANKI_ASSERT(e.m_handle.isCreated());
		return e.m_handle;
	}

	DescriptorHeapHandle getOrCreateDsv(const TextureSubresourceDescriptor& subresource, TextureUsageBit usage) const
	{
		const View& e = getOrCreateView(subresource, D3DTextureViewType::kDsv, usage);
		ANKI_ASSERT(e.m_handle.isCreated());
		return e.m_handle;
	}

	U32 calcD3DSubresourceIndex(const TextureSubresourceDescriptor& subresource) const
	{
		const TextureView view(this, subresource);
		if(view.isAllSurfacesOrVolumes() && view.getDepthStencilAspect() == m_aspect)
		{
			return D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		}
		else
		{
			const U32 faceCount = textureTypeIsCube(m_texType);
			const U32 arraySize = faceCount * m_layerCount;
			const U32 arraySlice = view.getFirstLayer() * faceCount + view.getFirstFace();
			const U32 planeSlice =
				(m_aspect == DepthStencilAspectBit::kDepthStencil && view.getDepthStencilAspect() == DepthStencilAspectBit::kStencil) ? 1 : 0;
			return view.getFirstMipmap() + (arraySlice * m_mipCount) + (planeSlice * m_mipCount * arraySize);
		}
	}

	/// By knowing the previous and new texture usage calculate the relavant info for a ppline barrier.
	void computeBarrierInfo(TextureUsageBit before, TextureUsageBit after, D3D12_RESOURCE_STATES& statesBefore,
							D3D12_RESOURCE_STATES& statesAfter) const;

	ID3D12Resource& getD3DResource() const
	{
		return *m_resource;
	}

private:
	class View
	{
	public:
		DescriptorHeapHandle m_handle;
		U32 m_bindlessIndex = kMaxU32;
		D3DTextureViewType m_viewType;
		Bool m_dsvReadOnly = false;
	};

	ID3D12Resource* m_resource = nullptr;

	mutable GrHashMap<U64, View> m_viewsMap;
	mutable RWMutex m_viewsMapMtx;

	// Cache a few common views
	View m_wholeTextureSrv;
	View m_firstSurfaceRtvOrDsv;
	TextureSubresourceDescriptor m_wholeTextureSrvSubresource = TextureSubresourceDescriptor::all();
	TextureSubresourceDescriptor m_firstSurfaceRtvOrDsvSubresource = TextureSubresourceDescriptor::all();

	Error initInternal(ID3D12Resource* external, const TextureInitInfo& init);

	const View& getOrCreateView(const TextureSubresourceDescriptor& subresource, D3DTextureViewType viewType, TextureUsageBit usage) const;

	void computeResourceStates(TextureUsageBit usage, D3D12_RESOURCE_STATES& states) const;

	DescriptorHeapHandle createDescriptorHeapHandle(const TextureSubresourceDescriptor& subresource, D3DTextureViewType viewType,
													Bool readOnlyDsv) const;

	Bool isExternal() const
	{
		return !!(m_usage & TextureUsageBit::kPresent);
	}
};
/// @}

} // end namespace anki
