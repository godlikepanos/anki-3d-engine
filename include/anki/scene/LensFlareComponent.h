// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/SceneNode.h"
#include "anki/Gr.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Lens flare scene component.
class LensFlareComponent final: public SceneComponent
{
public:
	LensFlareComponent(SceneNode* node)
		: SceneComponent(Type::LENS_FLARE, node)
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

	TexturePtr getTexture() const
	{
		return m_tex->getGrTexture();
	}

	OcclusionQueryPtr& getOcclusionQueryToTest();

	const ResourceGroupPtr& getResourceGroup() const
	{
		return m_rcGroup;
	}

	/// Get the occlusion query to test.
	/// @param[out] q The returned query.
	/// @param[out] queryInvalid It's true if the query has an old result that
	///             cannot be used.
	void getOcclusionQueryToCheck(
		OcclusionQueryPtr& q, Bool& queryInvalid);

	/// @name SceneComponent virtuals
	/// @{
	Error update(
		SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated) override
	{
		updated = false;
		return ErrorCode::NONE;
	}
	/// @}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::LENS_FLARE;
	}

private:
	TextureResourcePtr m_tex; ///< Array of textures.
	U8 m_flareCount = 0; ///< Cache the flare count.

	Vec4 m_colorMul = Vec4(1.0); ///< Color multiplier.

	Vec2 m_firstFlareSize = Vec2(1.0);
	Vec2 m_otherFlareSize = Vec2(1.0);

	Array<OcclusionQueryPtr, MAX_FRAMES_IN_FLIGHT> m_queries;
	Array<Timestamp, MAX_FRAMES_IN_FLIGHT> m_queryTestTimestamp =
		{{MAX_U32, MAX_U32, MAX_U32}};
	U8 m_crntQueryIndex = 0;

	Vec4 m_worldPosition = Vec4(0.0);

	ResourceGroupPtr m_rcGroup;
};
/// @}

} // end namespace anki

