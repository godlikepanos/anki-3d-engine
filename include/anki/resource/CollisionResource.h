// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_COLLISION_RESOURCE_H
#define ANKI_RESOURCE_COLLISION_RESOURCE_H

#include "anki/resource/ResourceObject.h"
#include "anki/resource/ResourcePointer.h"
#include "anki/physics/PhysicsCollisionShape.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Load a collision shape.
///
/// XML file format:
/// <collisionShape>
/// 	<type>sphere | box | mesh</type>
/// 	<value>radius | extend | path/to/mesh</value>
/// </collisionShape>
class CollisionResource: public ResourceObject
{
public:
	CollisionResource(ResourceManager* manager)
	:	ResourceObject(manager)
	{}

	~CollisionResource()
	{}

	ANKI_USE_RESULT Error load(const CString& filename);

	PhysicsCollisionShapePtr getShape() const
	{
		return m_physicsShape;
	}

private:
	PhysicsCollisionShapePtr m_physicsShape;
};
/// @}

} // end namespace anki

#endif

