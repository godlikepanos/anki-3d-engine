#ifndef ANKI_SCENE_FRUSTUMABLE_H
#define ANKI_SCENE_FRUSTUMABLE_H

#include "anki/collision/Frustum.h"


namespace anki {


/// @addtogroup Scene
/// @{

/// Frustumable "interface" for scene nodes
class Frustumable
{
public:
	/// Pass the frustum here so we can avoid the virtuals
	Frustumable(Frustum* fr)
		: frustum(fr)
	{}

	const Frustum& getFrustum() const
	{
		return *frustum;
	}
	Frustum& getFrustum()
	{
		return *frustum;
	}

	bool insideFrustum(const CollisionShape& cs) const
	{
		return frustum->insideFrustum(cs);
	}

private:
	Frustum* frustum;
};
/// @}


} // namespace anki


#endif
