// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
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

	DescriptorHeapHandle getOrCreateRtv(const TextureSubresourceDesc& subresource) const
	{
		const View& e = getOrCreateView(subresource, ViewType::kRtv);
		ANKI_ASSERT(e.m_handle.isCreated());
		return e.m_handle;
	}

	DescriptorHeapHandle getOrCreateDsv(const TextureSubresourceDesc& subresource, Bool readOnly) const
	{
		const View& e = getOrCreateView(subresource, (readOnly) ? ViewType::kReadOnlyDsv : ViewType::kDsv);
		ANKI_ASSERT(e.m_handle.isCreated());
		return e.m_handle;
	}

	DescriptorHeapHandle getOrCreateSrv(const TextureSubresourceDesc& subresource) const
	{
		const View& e = getOrCreateView(subresource, ViewType::kSrv);
		ANKI_ASSERT(e.m_handle.isCreated());
		return e.m_handle;
	}

	DescriptorHeapHandle getOrCreateUav(const TextureSubresourceDesc& subresource) const
	{
		const View& e = getOrCreateView(subresource, ViewType::kUav);
		ANKI_ASSERT(e.m_handle.isCreated());
		return e.m_handle;
	}

	U32 calcD3DSubresourceIndex(const TextureSubresourceDesc& subresource) const
	{
		const TextureView view(this, subresource);

		const U32 faceCount = textureTypeIsCube(m_texType);
		const U32 arraySize = faceCount * m_layerCount;
		const U32 arraySlice = view.getFirstLayer() * faceCount + view.getFirstFace();
		const U32 planeSlice =
			(m_aspect == DepthStencilAspectBit::kDepthStencil && view.getDepthStencilAspect() == DepthStencilAspectBit::kStencil) ? 1 : 0;
		return view.getFirstMipmap() + (arraySlice * m_mipCount) + (planeSlice * m_mipCount * arraySize);
	}

	D3D12_TEXTURE_BARRIER computeBarrierInfo(TextureUsageBit before, TextureUsageBit after, const TextureSubresourceDesc& subresource) const;

	ID3D12Resource& getD3DResource() const
	{
		return *m_resource;
	}

	DXGI_FORMAT getDxgiFormat() const
	{
		return convertFormat(m_format);
	}

private:
	enum class ViewType : U8
	{
		kNone,
		kRtv,
		kDsv,
		kReadOnlyDsv,
		kSrv,
		kUav
	};

	class View
	{
	public:
		DescriptorHeapHandle m_handle;

		mutable DescriptorHeapHandle m_bindlessHandle;
		mutable U32 m_bindlessIndex = kMaxU32;
		mutable SpinLock m_bindlessLock;

		ViewType m_type = ViewType::kNone;

		View() = default;

		View(const View& b)
		{
			*this = b;
		}

		View& operator=(const View& b)
		{
			m_handle = b.m_handle;
			m_bindlessHandle = b.m_bindlessHandle;
			m_bindlessIndex = b.m_bindlessIndex;
			m_type = b.m_type;
			return *this;
		}
	};

	ID3D12Resource* m_resource = nullptr;

	mutable GrHashMap<U64, View> m_viewsMap;
	mutable RWMutex m_viewsMapMtx;

	// Cache a few common views
	View m_wholeTextureSrv;
	View m_firstSurfaceRtvOrDsv;
	TextureSubresourceDesc m_wholeTextureSrvSubresource = TextureSubresourceDesc::all();
	TextureSubresourceDesc m_firstSurfaceRtvOrDsvSubresource = TextureSubresourceDesc::all();

	Error initInternal(ID3D12Resource* external, const TextureInitInfo& init);

	const View& getOrCreateView(const TextureSubresourceDesc& subresource, ViewType type) const;

	void initView(const TextureSubresourceDesc& subresource, ViewType type, View& view) const;

	void computeBarrierInfo(TextureUsageBit usage, D3D12_BARRIER_SYNC& stages, D3D12_BARRIER_ACCESS& accesses) const;

	D3D12_BARRIER_LAYOUT computeLayout(TextureUsageBit usage) const;

	Bool isExternal() const
	{
		return !!(m_usage & TextureUsageBit::kPresent);
	}
};
/// @}

} // end namespace anki
