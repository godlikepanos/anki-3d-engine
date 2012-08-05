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

	const VisibilityInfo& getVisibilityInfo() const
	{
		return vinfo;
	}
	VisibilityInfo& getVisibilityInfo()
	{
		return vinfo;
	}

	uint32_t getFrustumableLastUpdateFrame() const
	{
		return lastUpdateFrame;
	}
	/// @}

	/// Called when a frustum parameter changes
	virtual void frustumUpdate()
	{
		spatialMarkUpdated();
	}

	void spatialMarkUpdated()
	{
		lastUpdateFrame = SceneSingleton::get().getFramesCount();
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
	uint32_t lastUpdateFrame = SceneSingleton::get().getFramesCount();
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
	float getFovX() const
	{
		return get().getFovX();
	}
	void setFovX(float ang)
	{
		get().setFovX(ang);
		frustumUpdate();
	}

	float getFovY() const
	{
		return get().getFovY();
	}
	void setFovY(float ang)
	{
		get().setFovY(ang);
		frustumUpdate();
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(float fovX_, float fovY_, float near_, float far_)
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
	float getLeft() const
	{
		return get().getLeft();
	}
	void setLeft(float f)
	{
		get().setLeft(f);
		frustumUpdate();
	}

	float getRight() const
	{
		return get().getRight();
	}
	void setRight(float f)
	{
		get().setRight(f);
		frustumUpdate();
	}

	float getTop() const
	{
		return get().getTop();
	}
	void setTop(float f)
	{
		get().setTop(f);
		frustumUpdate();
	}

	float getBottom() const
	{
		return get().getBottom();
	}
	void setBottom(float f)
	{
		get().setBottom(f);
		frustumUpdate();
	}

	/// Set all
	void setAll(float left_, float right_, float near_,
		float far_, float top_, float bottom_)
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

} // namespace anki

#endif
