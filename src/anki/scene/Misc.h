// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SpatialComponent.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// A class that holds spatial information and implements the SpatialComponent virtuals. You just need to update the
/// OBB manually
class ObbSpatialComponent : public SpatialComponent
{
public:
	Obb m_obb;

	ObbSpatialComponent(SceneNode* node)
		: SpatialComponent(node, &m_obb)
	{
	}
};
/// @}

} // end namespace anki
