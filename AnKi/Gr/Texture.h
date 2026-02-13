// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Buffer.h>

namespace anki {

// Forward
class TextureSubresourceDesc;

// Texture initializer.
class TextureInitInfo : public GrBaseInitInfo
{
public:
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 1; // Relevant only for 3D textures.
	U32 m_layerCount = 1; // Relevant only for texture arrays.

	Format m_format = Format::kNone;

	TextureUsageBit m_usage = TextureUsageBit::kNone; // How the texture will be used.
	TextureType m_type = TextureType::k2D;

	BufferView m_memoryBuffer; // Optionally provide the memory for the texture

	U8 m_mipmapCount = 1;

	U8 m_samples = 1;

	TextureInitInfo() = default;

	TextureInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	U64 computeHash() const
	{
		Array<U64, 9> arr;
		U32 count = 0;
		arr[count++] = m_width;
		arr[count++] = m_height;
		arr[count++] = m_depth;
		arr[count++] = m_layerCount;
		arr[count++] = U64(m_format);
		arr[count++] = U64(m_usage);
		arr[count++] = U64(m_type);
		arr[count++] = m_mipmapCount;
		arr[count++] = m_samples;
		return computeObjectHash(arr);
	}

	Bool isValid() const
	{
#define ANKI_CHECK_VAL_VALIDITY(x) \
	do \
	{ \
		if(!(x)) \
		{ \
			return false; \
		} \
	} while(0)

		ANKI_CHECK_VAL_VALIDITY(m_format != Format::kNone);
		ANKI_CHECK_VAL_VALIDITY(m_usage != TextureUsageBit::kNone);
		ANKI_CHECK_VAL_VALIDITY(m_mipmapCount > 0);
		ANKI_CHECK_VAL_VALIDITY(m_width > 0);
		ANKI_CHECK_VAL_VALIDITY(m_height > 0);
		switch(m_type)
		{
		case TextureType::k2D:
			ANKI_CHECK_VAL_VALIDITY(m_depth == 1);
			ANKI_CHECK_VAL_VALIDITY(m_layerCount == 1);
			break;
		case TextureType::kCube:
			ANKI_CHECK_VAL_VALIDITY(m_depth == 1);
			ANKI_CHECK_VAL_VALIDITY(m_layerCount == 1);
			break;
		case TextureType::k3D:
			ANKI_CHECK_VAL_VALIDITY(m_depth > 0);
			ANKI_CHECK_VAL_VALIDITY(m_layerCount == 1);
			ANKI_CHECK_VAL_VALIDITY(!(m_usage & TextureUsageBit::kAllRtvDsv));
			break;
		case TextureType::k2DArray:
		case TextureType::kCubeArray:
			ANKI_CHECK_VAL_VALIDITY(m_depth == 1);
			ANKI_CHECK_VAL_VALIDITY(m_layerCount > 0);
			break;
		default:
			ANKI_CHECK_VAL_VALIDITY(0);
		};

		return true;
#undef ANKI_CHECK_VAL_VALIDITY
	}
};

// GPU texture.
class Texture : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kTexture;

	U32 getWidth() const
	{
		ANKI_ASSERT(m_width);
		return m_width;
	}

	U32 getHeight() const
	{
		ANKI_ASSERT(m_height);
		return m_height;
	}

	U32 getDepth() const
	{
		ANKI_ASSERT(m_depth);
		return m_depth;
	}

	U32 getLayerCount() const
	{
		ANKI_ASSERT(m_layerCount);
		return m_layerCount;
	}

	U8 getMipmapCount() const
	{
		ANKI_ASSERT(m_mipCount);
		return m_mipCount;
	}

	TextureType getTextureType() const
	{
		ANKI_ASSERT(m_texType != TextureType::kCount);
		return m_texType;
	}

	TextureUsageBit getTextureUsage() const
	{
		ANKI_ASSERT(!!m_usage);
		return m_usage;
	}

	Format getFormat() const
	{
		ANKI_ASSERT(m_format != Format::kNone);
		return m_format;
	}

	DepthStencilAspectBit getDepthStencilAspect() const
	{
		return m_aspect;
	}

	// Returns an index to be used for bindless access. Only for sampling.
	// It's thread-safe
	U32 getOrCreateBindlessTextureIndex(const TextureSubresourceDesc& subresource);

protected:
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_layerCount = 0;
	U8 m_mipCount = 0;
	TextureType m_texType = TextureType::kCount;
	TextureUsageBit m_usage = TextureUsageBit::kNone;
	Format m_format = Format::kNone;
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::kNone;

	// Construct.
	Texture(CString name)
		: GrObject(kClassType, name)
	{
	}

	// Destroy.
	~Texture()
	{
	}

private:
	// Allocate and initialize a new instance.
	[[nodiscard]] static Texture* newInstance(const TextureInitInfo& init);
};

// Defines a part of a texture. This part can be a single surface or volume or the whole texture.
class TextureSubresourceDesc
{
public:
	U16 m_layer = 0;
	U8 m_mipmap = 0;
	U8 m_face = 0;

	// This flag doesn't mean the whole texture unless the m_aspect is equal to the aspect of the Texture.
	Bool m_allSurfacesOrVolumes = true;

	DepthStencilAspectBit m_depthStencilAspect = DepthStencilAspectBit::kNone;

	U8 _m_padding[2] = {0, 0};

	constexpr TextureSubresourceDesc(const TextureSubresourceDesc&) = default;

	constexpr TextureSubresourceDesc& operator=(const TextureSubresourceDesc&) = default;

	constexpr Bool operator==(const TextureSubresourceDesc& b) const
	{
		return m_mipmap == b.m_mipmap && m_face == b.m_face && m_layer == b.m_layer && m_allSurfacesOrVolumes == b.m_allSurfacesOrVolumes
			   && m_depthStencilAspect == b.m_depthStencilAspect;
	}

	static constexpr TextureSubresourceDesc all(DepthStencilAspectBit aspect = DepthStencilAspectBit::kNone)
	{
		return TextureSubresourceDesc(0, 0, 0, true, aspect);
	}

	static constexpr TextureSubresourceDesc surface(U32 mip, U32 face, U32 layer, DepthStencilAspectBit aspect = DepthStencilAspectBit::kNone)
	{
		return TextureSubresourceDesc(mip, face, layer, false, aspect);
	}

	static constexpr TextureSubresourceDesc firstSurface(DepthStencilAspectBit aspect = DepthStencilAspectBit::kNone)
	{
		return TextureSubresourceDesc(0, 0, 0, false, aspect);
	}

	static constexpr TextureSubresourceDesc volume(U32 mip)
	{
		return TextureSubresourceDesc(mip, 0, 0, false, DepthStencilAspectBit::kNone);
	}

	// Returns true if there is a surface or volume that overlaps. It doesn't check the aspect.
	Bool overlapsWith(const TextureSubresourceDesc& b) const
	{
		return m_allSurfacesOrVolumes || b.m_allSurfacesOrVolumes || (m_mipmap == b.m_mipmap && m_face == b.m_face && m_layer == b.m_layer);
	}

	void validate([[maybe_unused]] const Texture& tex) const
	{
#if ANKI_ASSERTIONS_ENABLED
		if(!m_allSurfacesOrVolumes)
		{
			ANKI_ASSERT(m_mipmap <= tex.getMipmapCount());
			[[maybe_unused]] const U8 faceCount = textureTypeIsCube(tex.getTextureType()) ? 6 : 1;
			ANKI_ASSERT(m_face < faceCount);
			ANKI_ASSERT(m_layer < tex.getLayerCount());
		}
		else
		{
			ANKI_ASSERT(m_mipmap == 0 && m_face == 0 && m_layer == 0);
		}

		if(getFormatInfo(tex.getFormat()).m_depthStencil == DepthStencilAspectBit::kDepthStencil)
		{
			ANKI_ASSERT(!!m_depthStencilAspect);
		}
		else if(getFormatInfo(tex.getFormat()).m_depthStencil == DepthStencilAspectBit::kDepth)
		{
			ANKI_ASSERT(m_depthStencilAspect == DepthStencilAspectBit::kDepth);
		}
		else if(getFormatInfo(tex.getFormat()).m_depthStencil == DepthStencilAspectBit::kStencil)
		{
			ANKI_ASSERT(m_depthStencilAspect == DepthStencilAspectBit::kStencil);
		}
		else
		{
			ANKI_ASSERT(m_depthStencilAspect == DepthStencilAspectBit::kNone);
		}
#endif
	}

private:
	constexpr TextureSubresourceDesc(U32 mip, U32 face, U32 layer, Bool allSurfs, DepthStencilAspectBit aspect)
		: m_layer(layer & kMaxU16)
		, m_mipmap(mip & kMaxU8)
		, m_face(face & kMaxU8)
		, m_allSurfacesOrVolumes(allSurfs)
		, m_depthStencilAspect(aspect)
	{
		static_assert(sizeof(TextureSubresourceDesc) == 8, "Because it may get hashed");
	}
};

// Defines a part of a texture. This part can be a single surface or volume or the whole texture.
class TextureView
{
public:
	TextureView()
		: m_subresource(TextureSubresourceDesc::all())
	{
	}

	explicit TextureView(const Texture* tex, const TextureSubresourceDesc& subresource = TextureSubresourceDesc::all())
		: m_tex(tex)
		, m_subresource(subresource)
	{
		ANKI_ASSERT(tex);
		if(textureTypeIsCube(m_tex->getTextureType()))
		{
			m_subresource.m_allSurfacesOrVolumes = subresource.m_allSurfacesOrVolumes;
		}
		else
		{
			m_subresource.m_allSurfacesOrVolumes =
				(m_tex->getMipmapCount() == 1 && m_tex->getLayerCount() == 1) || subresource.m_allSurfacesOrVolumes;
		}

		// Sanitize a bit
		if(subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone && tex->getDepthStencilAspect() != DepthStencilAspectBit::kNone)
		{
			m_subresource.m_depthStencilAspect = tex->getDepthStencilAspect();
		}

		validate();
	}

	TextureView(const TextureView&) = default;

	TextureView& operator=(const TextureView&) = default;

	[[nodiscard]] Bool isValid() const
	{
		return m_tex != nullptr;
	}

	[[nodiscard]] const Texture& getTexture() const
	{
		validate();
		return *m_tex;
	}

	// Returns true if the view contains all surfaces or volumes. It's orthogonal to depth stencil aspect.
	[[nodiscard]] Bool isAllSurfacesOrVolumes() const
	{
		validate();
		return m_subresource.m_allSurfacesOrVolumes;
	}

	[[nodiscard]] DepthStencilAspectBit getDepthStencilAspect() const
	{
		validate();
		return m_subresource.m_depthStencilAspect;
	}

	[[nodiscard]] U32 getFirstMipmap() const
	{
		validate();
		return (m_subresource.m_allSurfacesOrVolumes) ? 0 : m_subresource.m_mipmap;
	}

	[[nodiscard]] U32 getMipmapCount() const
	{
		validate();
		return (m_subresource.m_allSurfacesOrVolumes) ? m_tex->getMipmapCount() : 1;
	}

	[[nodiscard]] U32 getFirstLayer() const
	{
		validate();
		return (m_subresource.m_allSurfacesOrVolumes) ? 0 : m_subresource.m_layer;
	}

	[[nodiscard]] U32 getLayerCount() const
	{
		validate();
		return (m_subresource.m_allSurfacesOrVolumes) ? m_tex->getLayerCount() : 1;
	}

	[[nodiscard]] U32 getFirstFace() const
	{
		validate();
		return (m_subresource.m_allSurfacesOrVolumes) ? 0 : m_subresource.m_face;
	}

	[[nodiscard]] U32 getFaceCount() const
	{
		validate();
		return (m_subresource.m_allSurfacesOrVolumes) ? (textureTypeIsCube(m_tex->getTextureType()) ? 6 : 1) : 1;
	}

	[[nodiscard]] Bool isGoodForSampling() const
	{
		validate();
		// Can bound only one aspect at a time.
		return (m_subresource.m_depthStencilAspect == DepthStencilAspectBit::kDepth
				|| m_subresource.m_depthStencilAspect == DepthStencilAspectBit::kStencil
				|| m_subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone)
			   && !!(m_tex->getTextureUsage() & TextureUsageBit::kAllSrv);
	}

	// Return true if the subresource can be used in CommandBuffer::copyBufferToTexture.
	[[nodiscard]] Bool isGoodForCopyBufferToTexture() const
	{
		validate();
		return isSingleSurfaceOrVolume() && m_subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone
			   && !!(m_tex->getTextureUsage() & TextureUsageBit::kCopyDestination);
	}

	[[nodiscard]] Bool isGoodForStorage() const
	{
		validate();
		return isSingleSurfaceOrVolume() && m_subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone
			   && !!(m_tex->getTextureUsage() & TextureUsageBit::kAllUav);
	}

	[[nodiscard]] Bool isGoodForRenderTarget() const
	{
		validate();
		return isSingleSurfaceOrVolume() && !!(m_tex->getTextureUsage() & TextureUsageBit::kAllRtvDsv);
	}

	// Returns true if there is a surface or volume that overlaps. It doesn't check the aspect.
	[[nodiscard]] Bool overlapsWith(const TextureView& b) const
	{
		validate();
		b.validate();
		ANKI_ASSERT(m_tex == b.m_tex);
		return m_subresource.overlapsWith(b.m_subresource);
	}

	const TextureSubresourceDesc& getSubresource() const
	{
		validate();
		return m_subresource;
	}

private:
	const Texture* m_tex = nullptr;
	TextureSubresourceDesc m_subresource;

	void validate() const
	{
		ANKI_ASSERT(m_tex);
		m_subresource.validate(*m_tex);
	}

	Bool isSingleSurfaceOrVolume() const
	{
		validate();

		Bool singleSurfaceOrVolume;
		if(textureTypeIsCube(m_tex->getTextureType()))
		{
			singleSurfaceOrVolume = !m_subresource.m_allSurfacesOrVolumes;
		}
		else
		{
			singleSurfaceOrVolume = (m_tex->getMipmapCount() == 1 && m_tex->getLayerCount() == 1) || !m_subresource.m_allSurfacesOrVolumes;
		}

		return singleSurfaceOrVolume;
	}
};

} // end namespace anki
