// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/physics/PhysicsCollisionShape.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Load a collision shape.
///
/// XML file format:
/// @code
/// <collisionShape>
/// 	<type>sphere | box | mesh</type>
/// 	<value>radius | extend | path/to/mesh</value>
/// </collisionShape>
/// @endcode
class CollisionResource : public ResourceObject
{
public:
	CollisionResource(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~CollisionResource()
	{
	}

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	PhysicsCollisionShapePtr getShape() const
	{
		return m_physicsShape;
	}

private:
	PhysicsCollisionShapePtr m_physicsShape;
};
/// @}

} // end namespace anki
