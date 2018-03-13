// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/CollisionResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MeshLoader.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/misc/Xml.h>

namespace anki
{

Error CollisionResource::load(const ResourceFilename& filename, Bool async)
{
	XmlElement el;
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement collEl;
	ANKI_CHECK(doc.getChildElement("collisionShape", collEl));

	ANKI_CHECK(collEl.getChildElement("type", el));
	CString type;
	ANKI_CHECK(el.getText(type));

	XmlElement valEl;
	ANKI_CHECK(collEl.getChildElement("value", valEl));

	PhysicsWorld& physics = getManager().getPhysicsWorld();
	PhysicsCollisionShapeInitInfo csInit;

	if(type == "sphere")
	{
		F64 tmp;
		ANKI_CHECK(valEl.getNumber(tmp));
		m_physicsShape = physics.newInstance<PhysicsSphere>(csInit, tmp);
	}
	else if(type == "box")
	{
		Vec3 extend;
		ANKI_CHECK(valEl.getVec3(extend));
		m_physicsShape = physics.newInstance<PhysicsBox>(csInit, extend);
	}
	else if(type == "staticMesh")
	{
		CString meshfname;
		ANKI_CHECK(valEl.getText(meshfname));

		MeshLoader loader(&getManager(), getTempAllocator());
		ANKI_CHECK(loader.load(meshfname));

		DynamicArrayAuto<U32> indices(getTempAllocator());
		DynamicArrayAuto<Vec3> positions(getTempAllocator());
		ANKI_CHECK(loader.storeIndicesAndPosition(indices, positions));

		m_physicsShape = physics.newInstance<PhysicsTriangleSoup>(
			csInit, &positions[0], sizeof(Vec3), &indices[0], indices.getSize());
	}
	else
	{
		ANKI_RESOURCE_LOGE("Incorrect collision type");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

} // end namespace anki
