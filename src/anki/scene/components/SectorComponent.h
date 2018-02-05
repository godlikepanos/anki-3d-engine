// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Dummy component to identify a portal.
class PortalComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::PORTAL;

	PortalComponent(SceneNode* node)
		: SceneComponent(CLASS_TYPE, node)
	{
	}
};

/// Dummy component to identify a sector.
class SectorComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::SECTOR;

	SectorComponent(SceneNode* node)
		: SceneComponent(CLASS_TYPE, node)
	{
	}
};
/// @}

} // end namespace anki
