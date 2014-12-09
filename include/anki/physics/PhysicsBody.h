// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PHYSICS_BODY_H
#define ANKI_PHYSICS_BODY_H

#include "anki/physics/Common.h"

namespace anki {

/// @addtogroup physics
/// @{

/// Rigid body.
class Body
{
public:
private:
	NewtonBody* m_body;
	Transform m_trf;
};
/// @}

} // end namespace anki

#endif
