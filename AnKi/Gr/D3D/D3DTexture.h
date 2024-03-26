// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Texture.h>

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

	Error init(const TextureInitInfo& inf);
};
/// @}

} // end namespace anki
