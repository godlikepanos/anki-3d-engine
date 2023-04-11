// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Resource/RenderingKey.h>

namespace anki {

/// @addtogroup scene
/// @{

/// This is the unique render state of a renderable object in the scene.
class RenderStateInfo
{
public:
	ShaderProgramPtr m_program;
	PrimitiveTopology m_primitiveTopology = PrimitiveTopology::kTriangles;
	Bool m_indexedDrawcall = true;
};

/// Holds an array of all render state buckets.
class RenderStateBucketContainer
{
public:
	~RenderStateBucketContainer();

	/// Add a new user for a specific render state and rendering technique.
	U32 addUser(const RenderStateInfo& state, RenderingTechnique technique);

	/// Remove the user.
	void removeUser(U32 bucketIndex, RenderingTechnique technique);

	template<typename TFunc>
	void interateBuckets(RenderingTechnique technique, TFunc func) const
	{
		for(const ExtendedBucket& b : m_buckets[technique])
		{
			if(b.m_userCount > 0)
			{
				func(static_cast<const RenderStateInfo&>(b));
			}
		}
	}

private:
	class ExtendedBucket : public RenderStateInfo
	{
	public:
		U64 m_hash = 0;
		U32 m_userCount = 0;
	};

	Array<SceneDynamicArray<ExtendedBucket>, U32(RenderingTechnique::kCount)> m_buckets;
	Mutex m_mtx;
};
/// @}

} // end namespace anki
