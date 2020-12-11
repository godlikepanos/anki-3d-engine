// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/CpuMeshResource.h>
#include <anki/resource/MeshBinaryLoader.h>

namespace anki
{

CpuMeshResource::CpuMeshResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

CpuMeshResource::~CpuMeshResource()
{
	m_indices.destroy(getAllocator());
	m_positions.destroy(getAllocator());
}

Error CpuMeshResource::load(const ResourceFilename& filename, Bool async)
{
	MeshBinaryLoader loader(&getManager());

	ANKI_CHECK(loader.load(filename));

	DynamicArrayAuto<Vec3> tempPositions(getAllocator());
	DynamicArrayAuto<U32> tempIndices(getAllocator());

	ANKI_CHECK(loader.storeIndicesAndPosition(tempIndices, tempPositions));

	m_indices = std::move(tempIndices);
	m_positions = std::move(tempPositions);

	return Error::NONE;
}

} // end namespace anki