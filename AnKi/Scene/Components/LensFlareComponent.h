// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Lens flare scene component.
class LensFlareComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LensFlareComponent)

public:
	LensFlareComponent(SceneNode* node);

	~LensFlareComponent();

	void loadImageResource(CString filename);

	Bool isEnabled() const
	{
		return m_image.isCreated();
	}

	CString getImageResourceFilename() const
	{
		return (m_image) ? m_image->getFilename() : CString();
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

	const Vec3& getWorldPosition() const
	{
		return m_worldPosition;
	}

	const ImageResource& getImage() const
	{
		return *m_image;
	}

private:
	static constexpr F32 kAabbSize = 25.0_cm;

	Vec4 m_colorMul = Vec4(1.0f); ///< Color multiplier.

	ImageResourcePtr m_image; ///< Array of textures.

	Vec2 m_firstFlareSize = Vec2(1.0f);
	Vec2 m_otherFlareSize = Vec2(1.0f);

	Vec3 m_worldPosition = Vec3(0.0f);

	Bool m_dirty = true;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
