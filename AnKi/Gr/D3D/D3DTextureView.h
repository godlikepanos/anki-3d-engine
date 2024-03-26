// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/TextureView.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Texture view implementation.
class TextureViewImpl final : public TextureView
{
public:
	TextureViewImpl(CString name)
		: TextureView(name)
	{
	}

	~TextureViewImpl()
	{
	}

	Error init(const TextureViewInitInfo& inf)
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}
};
/// @}

} // end namespace anki
