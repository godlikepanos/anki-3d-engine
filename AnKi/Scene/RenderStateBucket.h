// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Resource/RenderingKey.h>
#include <AnKi/Gr/ShaderProgram.h>

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

class RenderStateBucketIndex
{
	friend class RenderStateBucketContainer;

public:
	RenderStateBucketIndex() = default;

	RenderStateBucketIndex(const RenderStateBucketIndex&) = delete;

	RenderStateBucketIndex(RenderStateBucketIndex&& b)
	{
		*this = std::move(b);
	}

	~RenderStateBucketIndex();

	RenderStateBucketIndex& operator=(const RenderStateBucketIndex&) = delete;

	RenderStateBucketIndex& operator=(RenderStateBucketIndex&& b)
	{
		ANKI_ASSERT(!isValid() && "Forgot to delete");
		m_index = b.m_index;
		m_technique = b.m_technique;
		b.invalidate();
		return *this;
	}

	U32 get() const
	{
		ANKI_ASSERT(m_index != kMaxU32);
		return m_index;
	}

	Bool isValid() const
	{
		return m_index != kMaxU32;
	}

private:
	U32 m_index = kMaxU32;
	RenderingTechnique m_technique = RenderingTechnique::kCount;

	void invalidate()
	{
		m_index = kMaxU32;
		m_technique = RenderingTechnique::kCount;
	}
};

/// Holds an array of all render state buckets.
class RenderStateBucketContainer : public MakeSingleton<RenderStateBucketContainer>
{
	template<typename>
	friend class MakeSingleton;

public:
	/// Add a new user for a specific render state and rendering technique.
	/// @note It's thread-safe against addUser and removeUser
	RenderStateBucketIndex addUser(const RenderStateInfo& state, RenderingTechnique technique);

	/// Remove the user.
	/// @note It's thread-safe against addUser and removeUser
	void removeUser(RenderStateBucketIndex& bucketIndex);

	/// Iterate empty and non-empty buckets.
	template<typename TFunc>
	void iterateBuckets(RenderingTechnique technique, TFunc func) const
	{
		for(const ExtendedBucket& b : m_buckets[technique])
		{
			func(static_cast<const RenderStateInfo&>(b), b.m_userCount);
		}
	}

	/// Get the number of renderables of all the buckets of a specific rendering technique.
	U32 getBucketsItemCount(RenderingTechnique technique) const
	{
		return m_bucketItemCount[technique];
	}

	/// Get number of empty and non-empty buckets.
	U32 getBucketCount(RenderingTechnique technique) const
	{
		return m_buckets[technique].getSize();
	}

	/// Get number of non-empty buckets.
	U32 getActiveBucketCount(RenderingTechnique technique) const
	{
		return m_activeBucketCount[technique];
	}

private:
	class ExtendedBucket : public RenderStateInfo
	{
	public:
		U64 m_hash = 0;
		U32 m_userCount = 0;
	};

	Array<SceneDynamicArray<ExtendedBucket>, U32(RenderingTechnique::kCount)> m_buckets;
	Array<U32, U32(RenderingTechnique::kCount)> m_bucketItemCount = {};
	Array<U32, U32(RenderingTechnique::kCount)> m_activeBucketCount = {};

	Mutex m_mtx;

	RenderStateBucketContainer() = default;

	~RenderStateBucketContainer();
};

inline RenderStateBucketIndex::~RenderStateBucketIndex()
{
	RenderStateBucketContainer::getSingleton().removeUser(*this);
}
/// @}

} // end namespace anki
