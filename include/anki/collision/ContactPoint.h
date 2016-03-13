// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Collision test contact point
class ContactPoint
{
public:
	Vec4 m_position;
	Vec4 m_normal;
	F32 m_depth;
};

/// @}

} // end namespace anki
