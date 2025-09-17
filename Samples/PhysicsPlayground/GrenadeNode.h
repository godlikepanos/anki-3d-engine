// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/AnKi.h>
#include <Samples/PhysicsPlayground/Events.h>

using namespace anki;

class GrenadeNode : public SceneNode
{
public:
	GrenadeNode(CString name)
		: SceneNode(name)
	{
		setLocalScale(Vec3(2.8f));

		newComponent<MeshComponent>()->setMeshFilename("Assets/MESH_grenade_818651700502e14b.ankimesh");
		newComponent<MaterialComponent>()->setMaterialFilename("Assets/MTL_grenade_4346150e31bdb957.ankimtl");

		BodyComponent* bodyc = newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromMeshComponent);
		bodyc->setMass(1.0f);

		createDestructionEvent(this, 10.0_sec);
	}
};
