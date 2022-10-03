// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

CpuMeshResource::CpuMeshResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

CpuMeshResource::~CpuMeshResource()
{
	m_indices.destroy(getMemoryPool());
	m_positions.destroy(getMemoryPool());
}

Error CpuMeshResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	MeshBinaryLoader loader(&getManager());

	ANKI_CHECK(loader.load(filename));

	DynamicArrayRaii<Vec3> tempPositions(&getMemoryPool());
	DynamicArrayRaii<U32> tempIndices(&getMemoryPool());

	ANKI_CHECK(loader.storeIndicesAndPosition(tempIndices, tempPositions));

	m_indices = std::move(tempIndices);
	m_positions = std::move(tempPositions);

	// Create the collision shape
	const Bool convex = !!(loader.getHeader().m_flags & MeshBinaryFlag::kConvex);
	m_physicsShape = getManager().getPhysicsWorld().newInstance<PhysicsTriangleSoup>(m_positions, m_indices, convex);

	return Error::kNone;
}

} // end namespace anki
