#ifndef ANKI_SCENE_FRUSTUMABLE_H
#define ANKI_SCENE_FRUSTUMABLE_H

#include "anki/collision/Frustum.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/VisibilityTester.h"
#include "anki/scene/Common.h"

namespace anki {

// Forward
class SectorGroup;
class Sector;

/// @addtogroup Scene
/// @{

/// Frustumable interface for scene nodes
class Frustumable
{
	friend SectorGroup;
	friend Sector;

public:
	/// @name Constructors
	/// @{

	/// Pass the frustum here so we can avoid the virtuals
	Frustumable(const SceneAllocator<U8>& alloc, Frustum* fr)
		: frustum(fr), renderables(alloc), lights(alloc)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Frustum& getFrustum() const
	{
		return *frustum;
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
		ANKI_ASSERT(visibles != nullptr);
		return *visibles;
	}

	/// Get the origin for sorting and visibility tests
	virtual const Vec3& getFrustumableOrigin() const = 0;
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
	VisibilityInfo vinfo;
	Mat4 projectionMat = Mat4::getIdentity();
	Mat4 viewMat = Mat4::getIdentity();
	Mat4 viewProjectionMat = Mat4::getIdentity();

private:
	U32 timestamp = Timestamp::getTimestamp();

	/// Visibility stuff. It's per frame so the pointer is invalid on the next 
	/// frame and before any visibility tests are run
	VisibilityTestResults* visibles = nullptr;
};

/// @}

} // end namespace anki

#endif
