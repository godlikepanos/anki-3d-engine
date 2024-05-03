// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DDescriptorHeap.h>

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

	const DescriptorHeapHandle& getView(const TextureSubresourceDescriptor& subresource, D3DTextureViewType viewType) const
	{
		const TextureViewEntry& e = getViewEntry(subresource, viewType);
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
	class TextureViewEntry
	{
	public:
		DescriptorHeapHandle m_handle;

		mutable U32 m_bindlessIndex = kMaxU32;
		mutable SpinLock m_bindlessIndexLock;
	};

	ID3D12Resource* m_resource = nullptr;

	Array<GrDynamicArray<TextureViewEntry>, U32(D3DTextureViewType::kCount)> m_singleSurfaceOrVolumeViews;
	Array<TextureViewEntry, U32(D3DTextureViewType::kCount)> m_wholeTextureViews;

	Error initInternal(ID3D12Resource* external, const TextureInitInfo& init);

	const TextureViewEntry& getViewEntry(const TextureSubresourceDescriptor& subresource, D3DTextureViewType viewType) const;

	void computeResourceStates(TextureUsageBit usage, D3D12_RESOURCE_STATES& states) const;
};
/// @}

} // end namespace anki
