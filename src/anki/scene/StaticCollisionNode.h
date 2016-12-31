// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/resource/Forward.h>
#include <anki/physics/Forward.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Node that interacts with physics.
class StaticCollisionNode : public SceneNode
{
public:
	StaticCollisionNode(SceneGraph* scene, CString name);

	~StaticCollisionNode();

	/// Initialize the node.
	/// @param[in] resourceFname The file to load. It points to a .ankicl file.
	/// @param[in] transform The transformation. That cannot change.
	ANKI_USE_RESULT Error init(const CString& resourceFname, const Transform& transform);

private:
	CollisionResourcePtr m_rsrc;
	PhysicsBodyPtr m_body;
};
/// @}

} // end namespace anki
