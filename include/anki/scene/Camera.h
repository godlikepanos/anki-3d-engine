#ifndef ANKI_SCENE_CAMERA_H
#define ANKI_SCENE_CAMERA_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Frustumable.h"


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
	Camera(CameraType type_,
		const char* name, Scene* scene, // SceneNode
		uint movableFlags, Movable* movParent, // Movable
		Frustum* frustum) // Spatial &  Frustumable
		: SceneNode(name, scene), Movable(movableFlags, movParent, *this),
			Spatial(frustum), Frustumable(frustum), type(type_)
	{}

	virtual ~Camera();
	/// @}

	/// @name Accessors
	/// @{
	CameraType getCameraType() const
	{
		return type;
	}

	const Mat4& getProjectionMatrix() const
	{
		return projectionMat;
	}

	const Mat4& getInverseProjectionMatrix() const
	{
		return invProjectionMat;
	}

	const Mat4& getViewMatrix() const
	{
		return viewMat;
	}

	float getNear() const
	{
		return frustum->getNear();
	}
	void setNear(float x)
	{
		frustum->setNear(x);
		frustumUpdate();
	}

	float getFar() const
	{
		return frustum->getFar();
	}
	void setFar(float x)
	{
		frustum->setFar(x);
		frustumUpdate();
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::getFrustumable()
	Frustumable* getFrustumable()
	{
		return this;
	}

	/// Override SceneNode::getSpatial()
	Spatial* getSpatial()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate
	void frameUpdate(float prevUpdateTime, float crntTime, int frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
		Movable::update();
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update view matrix
	/// - Update frustum
	void movableUpdate()
	{
		Movable::movableUpdate();
		updateViewMatrix();
		getFrustum().transform(getWorldTransform());
	}
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Implements Frustumable::frustumUpdate(). Calculate the projection
	/// matrix
	void frustumUpdate()
	{
		projectionMat = getFrustum().calculateProjectionMatrix();
		invProjectionMat = projectionMat.getInverse();
	}
	/// @}

	void lookAtPoint(const Vec3& point);

private:
	/// @name Matrices
	/// @{
	Mat4 projectionMat;
	Mat4 viewMat;

	/// Used in deferred shading for the calculation of view vector (see
	/// CalcViewVector). The reason we store this matrix here is that we
	/// don't want it to be re-calculated all the time but only when the
	/// projection params (fovX, fovY, zNear, zFar) change. Fortunately
	/// the projection params change rarely. Note that the Camera as we all
	/// know re-calculates the matrices only when the parameters change!!
	Mat4 invProjectionMat;
	/// @}

	CameraType type;

	/// Calculate the @a viewMat
	///
	/// The view matrix is the inverse world transformation
	void updateViewMatrix()
	{
		viewMat = Mat4(getWorldTransform().getInverse());
	}
};


/// Perspective camera
class PerspectiveCamera: public Camera
{
public:
	ANKI_HAS_SLOTS(PerspectiveCamera)

	/// @name Constructors
	/// @{
	PerspectiveCamera(const char* name, Scene* scene,
		uint movableFlags, Movable* movParent);
	/// @}

	/// @name Accessors
	/// @{
	float getFovX() const
	{
		return frustum.getFovX();
	}
	void setFovX(float ang)
	{
		frustum.setFovX(ang);
		frustumUpdate();
	}

	float getFovY() const
	{
		return frustum.getFovY();
	}
	void setFovY(float ang)
	{
		frustum.setFovX(ang);
		frustumUpdate();
	}

	void setAll(float fovX_, float fovY_, float zNear_, float zFar_)
	{
		frustum.setAll(fovX_, fovY_, zNear_, zFar_);
		frustumUpdate();
	}
	/// @}

private:
	PerspectiveFrustum frustum;

	/// Called when the property changes
	void updateFrustumSlot(const PerspectiveFrustum&)
	{
		frustumUpdate();
	}
	ANKI_SLOT(updateFrustumSlot, const PerspectiveFrustum&)
};


/// Orthographic camera
class OrthographicCamera: public Camera
{
public:
	ANKI_HAS_SLOTS(OrthographicCamera)

	/// @name Constructors
	/// @{
	OrthographicCamera(const char* name, Scene* scene,
		uint movableFlags, Movable* movParent);
	/// @}

	/// @name Accessors
	/// @{
	float getLeft() const
	{
		return frustum.getLeft();
	}
	void setLeft(float f)
	{
		frustum.setLeft(f);
		frustumUpdate();
	}

	float getRight() const
	{
		return frustum.getRight();
	}
	void setRight(float f)
	{
		frustum.setRight(f);
		frustumUpdate();
	}

	float getTop() const
	{
		return frustum.getTop();
	}
	void setTop(float f)
	{
		frustum.setTop(f);
		frustumUpdate();
	}

	float getBottom() const
	{
		return frustum.getBottom();
	}
	void setBottom(float f)
	{
		frustum.setBottom(f);
		frustumUpdate();
	}

	/// Set all
	void setAll(float left_, float right_, float zNear_,
		float zFar_, float top_, float bottom_)
	{
		frustum.setAll(left_, right_, zNear_, zFar_, top_, bottom_);
		frustumUpdate();
	}
	/// @}

private:
	OrthographicFrustum frustum;

	void updateFrustumSlot(const OrthographicFrustum&)
	{
		frustumUpdate();
	}
	ANKI_SLOT(updateFrustumSlot, const OrthographicFrustum&)
};
/// @}


} // end namespace


#endif
