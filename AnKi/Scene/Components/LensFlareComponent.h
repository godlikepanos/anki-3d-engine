// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/TextureResource.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Lens flare scene component.
class LensFlareComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LensFlareComponent)

public:
	LensFlareComponent(SceneNode* node);

	~LensFlareComponent();

	ANKI_USE_RESULT Error loadTextureResource(CString textureFilename);

	Bool isLoaded() const
	{
		return m_tex.isCreated();
	}

	CString getTextureResourceFilename() const
	{
		return (m_tex) ? m_tex->getFilename() : CString();
	}

	void setWorldPosition(const Vec3& worldPosition)
	{
		m_worldPosition = worldPosition;
	}

	const Vec3& getWorldPosition() const
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
		el.m_worldPosition = m_worldPosition;
		el.m_firstFlareSize = m_firstFlareSize;
		el.m_colorMultiplier = m_colorMul;
		el.m_textureView = m_tex->getGrTextureView().get();
		el.m_userData = this;
		el.m_drawCallback = debugDrawCallback;
	}

private:
	Vec4 m_colorMul = Vec4(1.0f); ///< Color multiplier.

	SceneNode* m_node;
	TextureResourcePtr m_tex; ///< Array of textures.

	Vec2 m_firstFlareSize = Vec2(1.0f);
	Vec2 m_otherFlareSize = Vec2(1.0f);

	Vec3 m_worldPosition = Vec3(0.0f);

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// Do nothing
	}
};
/// @}

} // end namespace anki
