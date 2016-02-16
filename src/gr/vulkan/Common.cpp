// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/Common.h>

namespace anki 
{

//==============================================================================
VkCompareOp convertCompareOp(CompareOperation ak)
{
	VkCompareOp out = VK_COMPARE_OP_NEVER;
	switch(ak)
	{
	case CompareOperation::ALWAYS:
		out = VK_COMPARE_OP_ALWAYS;
		break;
	case CompareOperation::LESS:
		out = VK_COMPARE_OP_LESS;
		break;
	case CompareOperation::EQUAL:
		out = VK_COMPARE_OP_EQUAL;
		break;
	case CompareOperation::LESS_EQUAL:
		out = VK_COMPARE_OP_LESS_OR_EQUAL;
		break;
	case CompareOperation::GREATER:
		out = VK_COMPARE_OP_GREATER;
		break;
	case CompareOperation::GREATER_EQUAL:
		out = VK_COMPARE_OP_GREATER_OR_EQUAL;
		break;
	case CompareOperation::NOT_EQUAL:
		out = VK_COMPARE_OP_NOT_EQUAL;
		break;
	case CompareOperation::NEVER:
		out = VK_COMPARE_OP_NEVER;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

} // end namespace anki

