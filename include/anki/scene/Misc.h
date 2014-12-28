// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_MISC_H
#define ANKI_SCENE_MISC_H

#include "anki/scene/SpatialComponent.h"

namespace anki {

/// @addtogroup scene
/// @{

/// A class that holds spatial information and implements the SpatialComponent
/// virtuals. You just need to update the OBB manually
class ObbSpatialComponent: public SpatialComponent
{
public:
	Obb m_obb;

	ObbSpatialComponent(SceneNode* node)
	:	SpatialComponent(node, &m_obb)
	{}
};
/// @}

} // end namespace anki

#endif
