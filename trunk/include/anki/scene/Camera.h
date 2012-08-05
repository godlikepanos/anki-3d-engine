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
class Camera: public SceneNode, public Movable, public Spatial
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
		Frustum* frustum) // Spatial
		: SceneNode(name, scene), Movable(movableFlags, movParent, *this),
			Spatial(frustum), type(type_)
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

	const Mat4& getViewProjectionMatrix() const
	{
		return viewProjectionMat;
	}

	/// Needed by the renderer
	virtual float getNear() const = 0;
	/// Needed by the renderer
	virtual float getFar() const = 0;
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
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
	}
	/// @}

	void lookAtPoint(const Vec3& point);

protected:
	Mat4 projectionMat = Mat4::getIdentity();

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

	Mat4 viewMat = Mat4::getIdentity();
	Mat4 viewProjectionMat = Mat4::getIdentity();
};

/// Perspective camera
class PerspectiveCamera: public Camera, public PerspectiveFrustumable
{
public:
	ANKI_HAS_SLOTS(PerspectiveCamera)

	/// @name Constructors
	/// @{
	PerspectiveCamera(const char* name, Scene* scene,
		uint movableFlags, Movable* movParent);
	/// @}

	float getNear() const
	{
		return frustum.getNear();
	}

	float getFar() const
	{
		return frustum.getFar();
	}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getFrustumable()
	Frustumable* getFrustumable()
	{
		return this;
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
		spatialMarkUpdated();
	}
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Overrides Frustumable::frustumUpdate(). Calculate the projection
	/// matrix
	void frustumUpdate()
	{
		Frustumable::frustumUpdate();
		projectionMat = frustum.calculateProjectionMatrix();
		invProjectionMat = projectionMat.getInverse();
		updateViewProjectionMatrix();
		spatialMarkUpdated();
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
class OrthographicCamera: public Camera, public OrthographicFrustumable
{
public:
	ANKI_HAS_SLOTS(OrthographicCamera)

	/// @name Constructors
	/// @{
	OrthographicCamera(const char* name, Scene* scene,
		uint movableFlags, Movable* movParent);
	/// @}

	float getNear() const
	{
		return frustum.getNear();
	}

	float getFar() const
	{
		return frustum.getFar();
	}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getFrustumable()
	Frustumable* getFrustumable()
	{
		return this;
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

	/// @name Frustumable virtuals
	/// @{

	/// Overrides Frustumable::frustumUpdate(). Calculate the projection
	/// matrix
	void frustumUpdate()
	{
		Frustumable::frustumUpdate();
		projectionMat = frustum.calculateProjectionMatrix();
		invProjectionMat = projectionMat.getInverse();
		updateViewProjectionMatrix();
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

} // end namespace anki

#endif
