#ifndef ANKI_SCENE_FRUSTUMABLE_H
#define ANKI_SCENE_FRUSTUMABLE_H

#include "anki/collision/Frustum.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/VisibilityTester.h"

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

	F32 getNear() const
	{
		return frustum->getNear();
	}
	void setNear(const F32 x)
	{
		frustum->setNear(x);
		frustumUpdate();
	}

	F32 getFar() const
	{
		return frustum->getFar();
	}
	void setFar(const F32 x)
	{
		frustum->setFar(x);
		frustumUpdate();
	}

	const VisibilityInfo& getVisibilityInfo() const
	{
		return vinfo;
	}
	VisibilityInfo& getVisibilityInfo()
	{
		return vinfo;
	}

	U32 getFrustumableTimestamp() const
	{
		return timestamp;
	}
	/// @}

	/// Called when a frustum parameter changes
	virtual void frustumUpdate()
	{
		frustumableMarkUpdated();
	}

	void frustumableMarkUpdated()
	{
		timestamp = Timestamp::getTimestamp();
	}

	/// Is a spatial inside the frustum?
	bool insideFrustum(const Spatial& sp) const
	{
		return frustum->insideFrustum(sp.getSpatialCollisionShape());
	}

	/// Is a collision shape inside the frustum?
	bool insideFrustum(const CollisionShape& cs) const
	{
		return frustum->insideFrustum(cs);
	}

protected:
	Frustum* frustum = nullptr;
	VisibilityInfo vinfo;

private:
	U32 timestamp = Timestamp::getTimestamp();
};

/// Perspective prustumable interface for scene nodes
class PerspectiveFrustumable: public Frustumable
{
public:
	/// @name Constructors
	/// @{

	/// Pass the frustum here so we can avoid the virtuals
	PerspectiveFrustumable(PerspectiveFrustum* fr)
		: Frustumable(fr)
	{}
	/// @}

	/// @name Accessors
	/// @{
	F32 getFovX() const
	{
		return get().getFovX();
	}
	void setFovX(F32 ang)
	{
		get().setFovX(ang);
		frustumUpdate();
	}

	F32 getFovY() const
	{
		return get().getFovY();
	}
	void setFovY(F32 ang)
	{
		get().setFovY(ang);
		frustumUpdate();
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(F32 fovX_, F32 fovY_, F32 near_, F32 far_)
	{
		get().setAll(fovX_, fovY_, near_, far_);
		frustumUpdate();
	}
	/// @}

private:
	PerspectiveFrustum& get()
	{
		return *static_cast<PerspectiveFrustum*>(frustum);
	}
	const PerspectiveFrustum& get() const
	{
		return *static_cast<const PerspectiveFrustum*>(frustum);
	}
};

/// Orthographic prustumable interface for scene nodes
class OrthographicFrustumable: public Frustumable
{
public:
	/// @name Constructors
	/// @{

	/// Pass the frustum here so we can avoid the virtuals
	OrthographicFrustumable(OrthographicFrustum* fr)
		: Frustumable(fr)
	{}
	/// @}

	/// @name Accessors
	/// @{
	F32 getLeft() const
	{
		return get().getLeft();
	}
	void setLeft(F32 f)
	{
		get().setLeft(f);
		frustumUpdate();
	}

	F32 getRight() const
	{
		return get().getRight();
	}
	void setRight(F32 f)
	{
		get().setRight(f);
		frustumUpdate();
	}

	F32 getTop() const
	{
		return get().getTop();
	}
	void setTop(F32 f)
	{
		get().setTop(f);
		frustumUpdate();
	}

	F32 getBottom() const
	{
		return get().getBottom();
	}
	void setBottom(F32 f)
	{
		get().setBottom(f);
		frustumUpdate();
	}

	/// Set all
	void setAll(F32 left_, F32 right_, F32 near_,
		F32 far_, F32 top_, F32 bottom_)
	{
		get().setAll(left_, right_, near_, far_, top_, bottom_);
		frustumUpdate();
	}
	/// @}

private:
	OrthographicFrustum& get()
	{
		return *static_cast<OrthographicFrustum*>(frustum);
	}
	const OrthographicFrustum& get() const
	{
		return *static_cast<const OrthographicFrustum*>(frustum);
	}
};
/// @}

} // end namespace anki

#endif
