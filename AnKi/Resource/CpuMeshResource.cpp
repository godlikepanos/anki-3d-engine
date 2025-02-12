// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics2/PhysicsWorld.h>

namespace anki {

Error CpuMeshResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	MeshBinaryLoader loader(&ResourceMemoryPool::getSingleton());

	ANKI_CHECK(loader.load(filename));
	ANKI_CHECK(loader.storeIndicesAndPosition(loader.getHeader().m_lodCount - 1, m_indicesMaxLod, m_positionsMaxLod));

	m_isConvex = !!(loader.getHeader().m_flags & MeshBinaryFlag::kConvex);
	m_maxLod = U8(loader.getHeader().m_lodCount - 1);

	return Error::kNone;
}

const v2::PhysicsCollisionShapePtr& CpuMeshResource::getOrCreateCollisionShape(Bool isStatic, U32 lod) const
{
	lod = min<U32>(lod, m_maxLod);
	ANKI_ASSERT(lod == m_maxLod && "Not supported ATM");

	LockGuard lock(m_shapeMtx);

	if(!m_collisionShape)
	{
		if(m_isConvex || !isStatic)
		{
			if(!m_isConvex && !isStatic)
			{
				ANKI_RESOURCE_LOGE("Requesting a non-static non-convex collision shape is not supported. Will create a convex hull instead");
			}

			m_collisionShape = v2::PhysicsWorld::getSingleton().newConvexHullShape(m_positionsMaxLod);
		}
		else
		{
			m_collisionShape = v2::PhysicsWorld::getSingleton().newStaticMeshShape(m_positionsMaxLod, m_indicesMaxLod);
		}
	}

	return m_collisionShape;
}

} // end namespace anki
