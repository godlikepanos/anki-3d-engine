// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>

namespace anki {

/// @addtogroup scene
/// @{

class RenderStateInfo
{
public:
	ShaderProgramPtr m_program;
	PrimitiveTopology m_primitiveTopology;
	Bool m_indexed;
};

/// TODO
class RenderStateBucketContainer
{
public:
	U32 addUser(const RenderStateInfo& state);

	void removeUser(U32 bucketIndex);

private:
	class ExtendedBucket : public RenderStateInfo
	{
	public:
		U64 m_hash = 0;
		U32 m_userCount = 0;
	};
};
/// @}

} // end namespace anki
