#ifndef ANKI_SCENE_CAMERA_H
#define ANKI_SCENE_CAMERA_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/FrustumComponent.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class Camera: public SceneNode, public MoveComponent, public SpatialComponent, 
	public FrustumComponent
{
public:
	/// @note Don't EVER change the order
	enum CameraType
	{
		CT_PERSPECTIVE,
		CT_ORTHOGRAPHIC,
		CT_COUNT
	};

	/// @name Constructors/Destructor
	/// @{
	Camera(
		const char* name, SceneGraph* scene, // SceneNode
		Frustum* frustum, // Spatial & Frustumable
		CameraType type); // Self

	virtual ~Camera();
	/// @}

	/// @name Accessors
	/// @{
	CameraType getCameraType() const
	{
		return type;
	}

	const Mat4& getInverseProjectionMatrix() const
	{
		return invProjectionMat;
	}

	/// Needed by the renderer
	virtual F32 getNear() const = 0;
	/// Needed by the renderer
	virtual F32 getFar() const = 0;
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Override Frustumable::getFrustumableOrigin()
	const Vec3& getFrustumOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

	/// @name Spatial virtuals
	/// @{

	/// Override Spatial::getSpatialOrigin
	const Vec3& getSpatialOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

	void lookAtPoint(const Vec3& point);

protected:
	/// Used in deferred shading for the calculation of view vector (see
	/// CalcViewVector). The reason we store this matrix here is that we
	/// don't want it to be re-calculated all the time but only when the
	/// projection params (fovX, fovY, zNear, zFar) change. Fortunately
	/// the projection params change rarely. Note that the Camera as we all
	/// know re-calculates the matrices only when the parameters change!!
	Mat4 invProjectionMat = Mat4::getIdentity();

	/// Calculate the @a viewMat. The view matrix is the inverse world 
	/// transformation
	void updateViewMatrix()
	{
		viewMat = Mat4(getWorldTransform().getInverse());
	}

	void updateViewProjectionMatrix()
	{
		viewProjectionMat = projectionMat * viewMat;
	}

private:
	CameraType type;
};

/// Perspective camera
class PerspectiveCamera: public Camera
{
public:
	/// @name Constructors
	/// @{
	PerspectiveCamera(const char* name, SceneGraph* scene);
	/// @}

	/// @name Accessors
	/// @{
	F32 getNear() const
	{
		return frustum.getNear();
	}

	F32 getFar() const
	{
		return frustum.getFar();
	}

	F32 getFovX() const
	{
		return frustum.getFovX();
	}
	void setFovX(F32 x)
	{
		frustum.setFovX(x);
		frustumUpdate();
	}

	F32 getFovY() const
	{
		return frustum.getFovY();
	}
	void setFovY(F32 x)
	{
		frustum.setFovY(x);
		frustumUpdate();
	}

	void setAll(F32 fovX_, F32 fovY_, F32 near_, F32 far_)
	{
		frustum.setAll(fovX_, fovY_, near_, far_);
		frustumUpdate();
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// @li Update view matrix
	/// @li Update view-projection matrix
	/// @li Move the frustum
	void moveUpdate()
	{
		updateViewMatrix();
		updateViewProjectionMatrix();
		frustum.setTransform(getWorldTransform());

		SpatialComponent::markForUpdate();
	}
	/// @}

private:
	PerspectiveFrustum frustum;

	/// Called when something changes in the frustum
	void frustumUpdate()
	{
		projectionMat = frustum.calculateProjectionMatrix();
		invProjectionMat = projectionMat.getInverse();
		updateViewProjectionMatrix();

		SpatialComponent::markForUpdate();
		FrustumComponent::markForUpdate();
	}
};

/// Orthographic camera
class OrthographicCamera: public Camera
{
public:
	/// @name Constructors
	/// @{
	OrthographicCamera(const char* name, SceneGraph* scene);
	/// @}

	/// @name Accessors
	/// @{
	F32 getNear() const
	{
		return frustum.getNear();
	}

	F32 getFar() const
	{
		return frustum.getFar();
	}

	F32 getLeft() const
	{
		return frustum.getLeft();
	}

	F32 getRight() const
	{
		return frustum.getRight();
	}

	F32 getBottom() const
	{
		return frustum.getBottom();
	}

	F32 getTop() const
	{
		return frustum.getTop();
	}

	void setAll(F32 left, F32 right, F32 near, F32 far, F32 top, F32 bottom)
	{
		frustum.setAll(left, right, near, far, top, bottom);
		frustumUpdate();
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// @li Update view matrix
	/// @li Update view-projection matrix
	/// @li Update frustum
	void movableUpdate()
	{
		ANKI_ASSERT(0 && "TODO");
	}
	/// @}

private:
	OrthographicFrustum frustum;

	/// Called when something changes in the frustum
	void frustumUpdate()
	{
		ANKI_ASSERT(0 && "TODO");
	}
};
/// @}

} // end namespace anki

#endif
