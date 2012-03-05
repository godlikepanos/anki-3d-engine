#ifndef ANKI_SCENE_FRUSTUMABLE_H
#define ANKI_SCENE_FRUSTUMABLE_H

#include "anki/collision/Frustum.h"
#include "anki/scene/Spatial.h"


namespace anki {


/// @addtogroup Scene
/// @{

/// Frustumable interface for scene nodes
class Frustumable
{
public:
	/// @name Constructors
	/// @{

	/// Pass the frustum here so we can avoid the virtuals
	Frustumable(Frustum* fr)
		: frustum(fr)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Frustum& getFrustum() const
	{
		return *frustum;
	}
	Frustum& getFrustum()
	{
		return *frustum;
	}

	float getNear() const
	{
		return frustum->getNear();
	}
	void setNear(const float x)
	{
		frustum->setNear(x);
		frustumUpdate();
	}

	float getFar() const
	{
		return frustum->getFar();
	}
	void setFar(const float x)
	{
		frustum->setFar(x);
		frustumUpdate();
	}
	/// @}

	/// Called when a frustum parameter changes
	virtual void frustumUpdate() = 0;

	/// Is a spatial inside the frustum
	bool insideFrustum(const Spatial& sp) const
	{
		return frustum->insideFrustum(sp.getSpatialCollisionShape());
	}

protected:
	Frustum* frustum;
};


} // namespace anki


#endif
