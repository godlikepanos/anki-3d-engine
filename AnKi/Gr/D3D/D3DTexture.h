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

	Error initExternal(ID3D12Resource* image, const TextureInitInfo& init)
	{
		return initInternal(image, init);
	}

private:
	Error initInternal(ID3D12Resource* image, const TextureInitInfo& init);
};
/// @}

} // end namespace anki
