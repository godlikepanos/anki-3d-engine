#ifndef ANKI_SCENE_FRUSTUMABLE_H
#define ANKI_SCENE_FRUSTUMABLE_H

#include "anki/collision/Frustum.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Common.h"

namespace anki {

// Forward
class SectorGroup;
class Sector;
struct VisibilityTestResults;

/// @addtogroup Scene
/// @{

/// Frustumable interface for scene nodes
class Frustumable
{
	friend class SectorGroup;
	friend class Sector;

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

	U32 getFrustumableTimestamp() const
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

	/// Call this after the tests. Before it will point to junk
	const VisibilityTestResults& getVisibilityTestResults() const
	{
		ANKI_ASSERT(visible != nullptr);
		return *visible;
	}

	/// Get the origin for sorting and visibility tests
	virtual const Vec3& getFrustumableOrigin() const = 0;

	void setVisibilityTestResults(VisibilityTestResults* visible_)
	{
		visible = visible_;
	}
	VisibilityTestResults* getVisibilityTestResults()
	{
		return visible;
	}
	/// @}

	void frustumableMarkUpdated()
	{
		timestamp = Timestamp::getTimestamp();
	}

	/// Is a spatial inside the frustum?
	Bool insideFrustum(const Spatial& sp) const
	{
		return frustum->insideFrustum(sp.getSpatialCollisionShape());
	}

	/// Is a collision shape inside the frustum?
	Bool insideFrustum(const CollisionShape& cs) const
	{
		return frustum->insideFrustum(cs);
	}

protected:
	Frustum* frustum = nullptr;
	Mat4 projectionMat = Mat4::getIdentity();
	Mat4 viewMat = Mat4::getIdentity();
	Mat4 viewProjectionMat = Mat4::getIdentity();

private:
	U32 timestamp = Timestamp::getTimestamp();

	/// Visibility stuff. It's per frame so the pointer is invalid on the next 
	/// frame and before any visibility tests are run
	VisibilityTestResults* visible = nullptr;
};

/// @}

} // end namespace anki

#endif
