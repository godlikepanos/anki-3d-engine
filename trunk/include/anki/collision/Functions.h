#ifndef ANKI_COLLISION_FUNCTIONS_H
#define ANKI_COLLISION_FUNCTIONS_H

#include "anki/collision/Plane.h"
#include "anki/collision/Frustum.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Extract the clip planes using an MVP matrix
///
/// @param[in] mvp The MVP matrix.
/// @param[out] planes Pointers to the planes. Elements can be nullptr
///
/// @note plane_count * 8 muls, plane_count sqrt
extern void extractClipPlanes(const Mat4& mvp, 
	Plane* planes[(U)Frustum::PlaneType::COUNT]);

/// @}

} // end namespace anki

#endif
