// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Lens flare scene component.
class LensFlareComponent final : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::LENS_FLARE;

	LensFlareComponent(SceneNode* node);

	~LensFlareComponent();

	ANKI_USE_RESULT Error init(const CString& textureFilename);

	void setWorldPosition(const Vec4& worldPosition)
	{
		m_worldPosition = worldPosition;
	}

	const Vec4& getWorldPosition() const
	{
		return m_worldPosition;
	}

	void setFirstFlareSize(const Vec2& size)
	{
		m_firstFlareSize = size;
	}

	const Vec2& getFirstFlareSize() const
	{
		return m_firstFlareSize;
	}

	void setOtherFlareSize(const Vec2& size)
	{
		m_otherFlareSize = size;
	}

	const Vec2& getOtherFlareSize() const
	{
		return m_otherFlareSize;
	}

	void setColorMultiplier(const Vec4& color)
	{
		m_colorMul = color;
	}

	const Vec4& getColorMultiplier() const
	{
		return m_colorMul;
	}

	TexturePtr getTexture() const
	{
		return m_tex->getGrTexture();
	}

	void setupLensFlareQueueElement(LensFlareQueueElement& el) const
	{
		el.m_worldPosition = m_worldPosition.xyz();
		el.m_firstFlareSize = m_firstFlareSize;
		el.m_colorMultiplier = m_colorMul;
		el.m_textureView = m_tex->getGrTextureView().get();
		el.m_userData = this;
		el.m_drawCallback = debugDrawCallback;
	}

private:
	SceneNode* m_node;
	TextureResourcePtr m_tex; ///< Array of textures.

	Vec4 m_colorMul = Vec4(1.0); ///< Color multiplier.

	Vec2 m_firstFlareSize = Vec2(1.0);
	Vec2 m_otherFlareSize = Vec2(1.0);

	Vec4 m_worldPosition = Vec4(0.0);

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// Do nothing
	}
};
/// @}

} // end namespace anki
