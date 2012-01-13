#ifndef ANKI_SCENE_CAMERA_H
#define ANKI_SCENE_CAMERA_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Spartial.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Frustumable.h"
#include <boost/array.hpp>
#include <deque>
#include <vector>


namespace anki {


/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class Camera: public SceneNode, public Movable, public Spartial,
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

	Camera(CameraType type,
		const char* name, Scene* scene,
		uint movableFlags, Movable* movParent,
		CollisionShape* spartCs,
		Frustum* frustum)
		: type(type_), SceneNode(name, scene),
			Movable(movableFlags, movParent), Spartial(spartCs),
			Frustumable(frustum)
	{}

	virtual ~Camera();

	/// @name Accessors
	/// @{
	CameraType getCameraType() const
	{
		return type;
	}
	/// @}

	/// @name

	void lookAtPoint(const Vec3& point);

	/// This does:
	/// - Update view matrix
	/// - Update frustum planes
	virtual void moveUpdate();

protected:

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
private:
	CameraType type;

	void updateViewMatrix();
};
/// @}


} // end namespace


#endif
