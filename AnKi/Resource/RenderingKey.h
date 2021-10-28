// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Util/Hash.h>

namespace anki
{

/// The AnKi passes visible to materials.
enum class RenderingTechnique : U8
{
	GBUFFER,
	FORWARD_SHADING,
	RT_SHADOWS,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderingTechnique)

enum class RenderingTechniqueBit : U8
{
	NONE = 0,
	GBUFFER = 1 << 0,
	FORWARD_SHADING = 1 << 1,
	RT_SHADOWS = 1 << 2
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderingTechniqueBit)

enum class RenderingSubTechnique : U8
{
	MAIN,
	SHADOW_MAPPING,
	EARLY_Z,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderingSubTechnique)

/// A key that is used to fetch stuff from various resources.
class RenderingKey
{
public:
	RenderingKey()
	{
		memset(this, 0, sizeof(*this)); // Zero it because it will be hashed
		m_renderingTechnique = RenderingTechnique::COUNT;
		m_renderingSubTechnique = RenderingSubTechnique::COUNT;
		m_lod = MAX_U8;
	}

	RenderingTechnique m_renderingTechnique;
	RenderingSubTechnique m_renderingSubTechnique;
	U8 m_lod;
	Bool m_skinned : 1;
	Bool m_velocity : 1;
};

static_assert(sizeof(RenderingKey) == 4, "Should be packed because it will be hashed");

} // end namespace anki
