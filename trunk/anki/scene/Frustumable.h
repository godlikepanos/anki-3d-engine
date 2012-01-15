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

	bool insideFrustum(const CollisionShape& cs) const
	{
		return frustum->insideFrustum(cs);
	}

private:
	Frustum* frustum;
};


/// XXX
/*class PerspectiveFrustumable: public Frustumable
{
public:
	/// @name Constructors
	/// @{
	PerspectiveFrustumable(PerspectiveFrustum* fr)
		: Frustumable(fr)
	{}
	/// @}

	/// @name Accessors
	/// @{
	float getFovX() const
	{
		return toPerpectiveFrustum()->getFovX();
	}
	void setFovX(float ang)
	{
		toPerpectiveFrustum()->setFovX(ang);
		frustumUpdate();
	}

	float getFovY() const
	{
		return toPerpectiveFrustum()->getFovY();
	}
	void setFovY(float ang)
	{
		toPerpectiveFrustum()->setFovY(ang);
		frustumUpdate();
	}
	/// @}

private:
	PerspectiveFrustum* toPerpectiveFrustum()
	{
		return static_cast<PerspectiveFrustum*>(frustum);
	}
	const PerspectiveFrustum* toPerpectiveFrustum() const
	{
		return static_cast<const PerspectiveFrustum*>(frustum);
	}
};*/
/// @}


} // namespace anki


#endif
