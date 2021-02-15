// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// CPU Mesh Resource. It contains the geometry packed in CPU buffers.
class CpuMeshResource : public ResourceObject
{
public:
	/// Default constructor
	CpuMeshResource(ResourceManager* manager);

	~CpuMeshResource();

	/// Load from a mesh file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	ConstWeakArray<Vec3> getPositions() const
	{
		return m_positions;
	}

	ConstWeakArray<U32> getIndices() const
	{
		return m_indices;
	}

	const PhysicsCollisionShapePtr& getCollisionShape() const
	{
		return m_physicsShape;
	}

private:
	DynamicArray<Vec3> m_positions;
	DynamicArray<U32> m_indices;
	PhysicsCollisionShapePtr m_physicsShape;
};
/// @}

} // namespace anki
