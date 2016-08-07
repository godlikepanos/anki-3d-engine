// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/TextureMisc.h>

namespace anki
{

static_assert(MAX_RESOURCE_GROUPS == 2, "Code bellow assumes that");

static const CString UPLOAD_R8G8B8_COMPUTE_SHADER = R"(
const uint WORKGROUP_SIZE_X = 4u;

layout(local_size_x = WORKGROUP_SIZE_X,
	local_size_y = 1,
	local_size_z = 1) in;
	
layout(push_constant) uniform pc0_ 
{
	uvec4 texSize;
} u_regs;

layout(std430, set = 2, binding = 0) buffer blk0_
{
	uint u_pixels[];
};

void main()
{
	// XXX
}

)";

//==============================================================================
Error TextureFallbackUploader::init()
{

	return ErrorCode::NONE;
}

} // end namespace anki