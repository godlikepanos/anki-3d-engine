// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/resource/RenderingKey.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// The ID of a buildin material variable
enum class BuiltinMaterialVariableId2 : U8
{
	NONE = 0,
	MODEL_VIEW_PROJECTION_MATRIX,
	MODEL_VIEW_MATRIX,
	MODEL_MATRIX,
	VIEW_PROJECTION_MATRIX,
	VIEW_MATRIX,
	NORMAL_MATRIX,
	ROTATION_MATRIX,
	CAMERA_ROTATION_MATRIX,
	CAMERA_POSITION,
	PREVIOUS_MODEL_VIEW_PROJECTION_MATRIX,
	GLOBAL_SAMPLER,
	COUNT
};
/// @}

} // end namespace anki
