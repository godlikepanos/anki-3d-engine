// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

Error CollisionResource::load(const ResourceFilename& filename)
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
		ANKI_CHECK(valEl.getF64(tmp));
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

		MeshLoader loader(&getManager());
		ANKI_CHECK(loader.load(meshfname));

		m_physicsShape = physics.newInstance<PhysicsTriangleSoup>(csInit,
			reinterpret_cast<const Vec3*>(loader.getVertexData()),
			loader.getVertexSize(),
			reinterpret_cast<const U16*>(loader.getIndexData()),
			loader.getHeader().m_totalIndicesCount);
	}
	else
	{
		ANKI_RESOURCE_LOGE("Incorrect collision type");
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
