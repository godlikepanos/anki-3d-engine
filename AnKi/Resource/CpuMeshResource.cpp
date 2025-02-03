// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

Error CpuMeshResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	MeshBinaryLoader loader(&ResourceMemoryPool::getSingleton());

	ANKI_CHECK(loader.load(filename));
	ANKI_CHECK(loader.storeIndicesAndPosition(0, m_indices, m_positions));

	m_isConvex = !!(loader.getHeader().m_flags & MeshBinaryFlag::kConvex);

	return Error::kNone;
}

const PhysicsCollisionShapePtr& CpuMeshResource::getOrCreateCollisionShape([[maybe_unused]] Bool isStatic) const
{
	LockGuard lock(m_shapeMtx);

	if(!m_collisionShape)
	{
		m_collisionShape = PhysicsWorld::getSingleton().newInstance<PhysicsTriangleSoup>(m_positions, m_indices, m_isConvex);
	}

	return m_collisionShape;
}

} // end namespace anki
