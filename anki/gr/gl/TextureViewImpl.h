// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/TextureView.h>
#include <anki/gr/gl/GlObject.h>
#include <anki/gr/gl/TextureImpl.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Texture view implementation.
class TextureViewImpl final : public TextureView, public GlObject
{
public:
	MicroTextureView m_view = {};
	TexturePtr m_tex; ///< Hold a reference.

	TextureViewImpl(GrManager* manager, CString name)
		: TextureView(manager, name)
	{
	}

	~TextureViewImpl()
	{
		m_glName = 0;
	}

	void preInit(const TextureViewInitInfo& inf);

	void init()
	{
		m_view = static_cast<const TextureImpl&>(*m_tex).getOrCreateView(getSubresource());
		m_glName = m_view.m_glName;
	}
};
/// @}

} // end namespace anki