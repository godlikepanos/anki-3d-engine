// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_LENS_FLARE_NODE_H
#define ANKI_SCENE_LENS_FLARE_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/Gl.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Lens flare scene component.
class LensFlareComponent final: public SceneComponent
{
public:
	LensFlareComponent(SceneNode* node)
	:	SceneComponent(Type::LENS_FLARE, node),
		m_node(node)
	{}

	ANKI_USE_RESULT Error create(const CString& textureFilename);

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

	GlTextureHandle getTexture() const
	{
		return m_tex->getGlTexture();
	}

	GlOcclusionQueryHandle& getOcclusionQueryToTest()
	{
		return m_queries[m_crntQueryIndex];
	}

	GlOcclusionQueryHandle& getOcclusionQueryToCheck()
	{
		return m_queries[(m_crntQueryIndex + 1) % m_queries.getSize()];
	}

	/// @name SceneComponent virtuals
	/// @{
	Error update(
		SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated)
	{
		// Move the query counter
		m_crntQueryIndex = getGlobTimestamp() % m_queries.getSize();
		updated = false;
		return ErrorCode::NONE;
	}
	/// @}

	static constexpr Type getClassType()
	{
		return Type::LENS_FLARE;
	}

private:
	TextureResourcePointer m_tex; ///< Array of textures.
	U8 m_flareCount = 0; ///< Cache the flare count.

	Vec4 m_colorMul = Vec4(1.0); ///< Color multiplier.
	
	Vec2 m_firstFlareSize = Vec2(1.0);
	Vec2 m_otherFlareSize = Vec2(1.0);

	Array<GlOcclusionQueryHandle, 3> m_queries;
	U8 m_crntQueryIndex = 0;
	
	Vec4 m_worldPosition = Vec4(0.0);
	SceneNode* m_node;
};
/// @}

} // end namespace anki

#endif

