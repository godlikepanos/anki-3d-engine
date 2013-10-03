#ifndef ANKI_SCENE_FRUSTUM_COMPONENT_H
#define ANKI_SCENE_FRUSTUM_COMPONENT_H

#include "anki/collision/Frustum.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/Common.h"

namespace anki {

// Forward
struct VisibilityTestResults;

/// @addtogroup Scene
/// @{

/// Frustum component interface for scene nodes. Useful for nodes that are 
/// frustums like cameras and lights
class FrustumComponent
{
public:
	/// @name Constructors
	/// @{

	/// Pass the frustum here so we can avoid the virtuals
	FrustumComponent(Frustum* fr)
		: frustum(fr)
	{
		ANKI_ASSERT(frustum);
	}
	/// @}

	/// @name Accessors
	/// @{
	const Frustum& getFrustum() const
	{
		return *frustum;
	}

	Timestamp getTimestamp() const
	{
		return timestamp;
	}

	const Mat4& getProjectionMatrix() const
	{
		return projectionMat;
	}

	const Mat4& getViewMatrix() const
	{
		return viewMat;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		return viewProjectionMat;
	}

	/// Get the origin for sorting and visibility tests
	virtual const Vec3& getFrustumOrigin() const = 0;

	void setVisibilityTestResults(VisibilityTestResults* visible_)
	{
		ANKI_ASSERT(visible == nullptr);
		visible = visible_;
	}
	/// Call this after the tests. Before it will point to junk
	VisibilityTestResults& getVisibilityTestResults()
	{
		ANKI_ASSERT(visible != nullptr);
		return *visible;
	}
	/// @}

	void markForUpdate()
	{
		timestamp = getGlobTimestamp();
	}

	/// Is a spatial inside the frustum?
	Bool insideFrustum(const SpatialComponent& sp) const
	{
		return frustum->insideFrustum(sp.getSpatialCollisionShape());
	}

	/// Is a collision shape inside the frustum?
	Bool insideFrustum(const CollisionShape& cs) const
	{
		return frustum->insideFrustum(cs);
	}

	void resetFrame()
	{
		visible = nullptr;
	}

protected:
	Frustum* frustum = nullptr;
	Mat4 projectionMat = Mat4::getIdentity();
	Mat4 viewMat = Mat4::getIdentity();
	Mat4 viewProjectionMat = Mat4::getIdentity();

private:
	Timestamp timestamp = getGlobTimestamp();

	/// Visibility stuff. It's per frame so the pointer is invalid on the next 
	/// frame and before any visibility tests are run
	VisibilityTestResults* visible = nullptr;
};
/// @}

} // end namespace anki

#endif
