#ifndef ANKI_SCENE_CAMERA_H
#define ANKI_SCENE_CAMERA_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Frustumable.h"
#include "anki/core/Logger.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class Camera: public SceneNode, public Movable, public Spatial, 
	public Frustumable
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
		const char* name, SceneGraph* scene, SceneNode* parent, // SceneNode
		U32 movableFlags, // Movable
		Frustum* frustum, // Spatial & Frustumable
		CameraType type_); // Self

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

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, int frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
	}
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Override Frustumable::getFrustumableOrigin()
	const Vec3& getFrustumableOrigin() const
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
	ANKI_HAS_SLOTS(PerspectiveCamera)

	/// @name Constructors
	/// @{
	PerspectiveCamera(
		const char* name, SceneGraph* scene, SceneNode* parent, // SceneNode
		U32 movableFlags); // Movable
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
		frustumUpdate();
		frustum.setFovX(x);
	}

	F32 getFovY() const
	{
		return frustum.getFovY();
	}
	void setFovY(F32 x)
	{
		frustumUpdate();
		frustum.setFovY(x);
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
	/// @li Update frustum
	void movableUpdate()
	{
		Movable::movableUpdate();
		updateViewMatrix();
		updateViewProjectionMatrix();
		frustum.setTransform(getWorldTransform());
		spatialMarkForUpdate();
	}
	/// @}

private:
	PerspectiveFrustum frustum;

	/// Called when something changes in the frustum
	void frustumUpdate()
	{
		frustumableMarkUpdated();

		projectionMat = frustum.calculateProjectionMatrix();
		invProjectionMat = projectionMat.getInverse();
		updateViewProjectionMatrix();
		spatialMarkForUpdate();
	}
};

/// Orthographic camera
class OrthographicCamera: public Camera
{
public:
	ANKI_HAS_SLOTS(OrthographicCamera)

	/// @name Constructors
	/// @{
	OrthographicCamera(
		const char* name, SceneGraph* scene, SceneNode* parent, // SceneNode
		U32 movableFlags); // Movable
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
		Movable::movableUpdate();
		updateViewMatrix();
		updateViewProjectionMatrix();
		frustum.setTransform(getWorldTransform());
	}
	/// @}

private:
	OrthographicFrustum frustum;

	/// Called when something changes in the frustum
	void frustumUpdate()
	{
		frustumableMarkUpdated();

		projectionMat = frustum.calculateProjectionMatrix();
		invProjectionMat = projectionMat.getInverse();
		updateViewProjectionMatrix();
		spatialMarkForUpdate();
	}
};
/// @}

} // end namespace anki

#endif