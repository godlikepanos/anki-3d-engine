// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Reflection probe component.
class ReflectionProbeComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::REFLECTION_PROBE;

	ReflectionProbeComponent(SceneNode* node)
		: SceneComponent(CLASS_TYPE, node)
	{
	}

	const Vec4& getPosition() const
	{
		return m_pos;
	}

	void setPosition(const Vec4& pos)
	{
		m_pos = pos.xyz0();
	}

	F32 getRadius() const
	{
		ANKI_ASSERT(m_radius > 0.0);
		return m_radius;
	}

	void setRadius(F32 radius)
	{
		ANKI_ASSERT(radius > 0.0);
		m_radius = radius;
	}

	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	void setMarkedForRendering(Bool render)
	{
		m_markedForRendering = render;
	}

	void setTextureArrayIndex(U idx)
	{
		m_textureArrayIndex = idx;
	}

	U getTextureArrayIndex() const
	{
		ANKI_ASSERT(m_textureArrayIndex < MAX_U16);
		return m_textureArrayIndex;
	}

private:
	Vec4 m_pos = Vec4(0.0);
	F32 m_radius = 0.0;
	Bool8 m_markedForRendering = false;

	U16 m_textureArrayIndex = MAX_U16; ///< Used by the renderer
};
/// @}

} // end namespace anki
