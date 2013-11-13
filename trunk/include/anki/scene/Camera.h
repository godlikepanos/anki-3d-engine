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
class Camera: public SceneNode
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

	/// Needed by the renderer
	virtual F32 getNear() const = 0;
	/// Needed by the renderer
	virtual F32 getFar() const = 0;
	/// @}

	/// @name SceneNode virtuals
	/// @{
	void componentUpdated(SceneComponent& comp, 
		SceneComponent::UpdateType) override
	{
		if(comp.getId() == SceneComponent::getVariadicTypeId<MoveComponent>())
		{
			moveUpdate(static_cast<MoveComponent&>(comp));
		}
	}
	/// @}

	void lookAtPoint(const Vec3& point);

protected:
	/// Called when something changes in the frustum
	void frustumUpdate();

	/// Called when the world transform got updated
	void moveUpdate(MoveComponent& move);

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

private:
	PerspectiveFrustum frustum;
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

private:
	OrthographicFrustum frustum;
};
/// @}

} // end namespace anki

#endif
