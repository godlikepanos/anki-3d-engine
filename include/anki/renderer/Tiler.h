// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_TILER_H
#define ANKI_RENDERER_TILER_H

#include "anki/Collision.h"
#include "anki/renderer/RenderingPass.h"
#include "anki/core/Timestamp.h"

namespace anki {

class Camera;
class ShaderProgramUniformVariable;
class Frustumable;

/// @addtogroup renderer
/// @{

/// The result of the Tiler tests.
struct TilerTestResult
{
	struct Pair
	{
#if ANKI_DEBUG
		U8 m_x = 0xFF;
		U8 m_y = 0xFF;
#else
		U8 m_x;
		U8 m_y;
#endif
	};

	U32 m_count = 0;
	Array<Pair, ANKI_RENDERER_MAX_TILES_X * ANKI_RENDERER_MAX_TILES_Y> 
		m_tileIds;

	TilerTestResult() = default;

	void pushBack(U x, U y)
	{
		ANKI_ASSERT(x < 0xFF && y < 0xFF);

		Pair p;
		p.m_x = x;
		p.m_y = y;
		m_tileIds[m_count++] = p;
	}
};

/// Test paramters passed to Tiler::test
struct TilerTestParameters
{
	const CollisionShape* m_collisionShape = nullptr;
	const Aabb* m_collisionShapeBox = nullptr;
	Bool m_nearPlane = false;
	TilerTestResult* m_output;
};

/// Tiler used for visibility tests
class Tiler: public RenderingPass
{
	friend class UpdatePlanesPerspectiveCameraTask;

public:
	using TestResult = TilerTestResult;
	using TestParameters = TilerTestParameters;

	Tiler(Renderer* r);
	~Tiler();

	ANKI_USE_RESULT Error init();

	/// Issue the GPU job
	void runMinMax(CommandBufferHandle& cmd);

	/// Update the tiles before doing visibility tests
	void updateTiles(Camera& cam);

	/// Test against all tiles.
	/// @param[in, out] params The collision parameters.
	Bool test(TestParameters& params) const;

	/// @privatesection
	/// @{
	BufferHandle& getTilesBuffer()
	{
		U i = (getGlobalTimestamp() - m_pbos.getSize() + 1) % m_pbos.getSize();
		return m_pbos[i];
	}
	/// @}

private:
	/// Tile planes
	DArray<Plane> m_allPlanes;
	SArray<Plane> m_planesY;
	SArray<Plane> m_planesX;
	SArray<Plane> m_planesYW;
	SArray<Plane> m_planesXW;
	SArray<Plane> m_nearPlanesW;
	SArray<Plane> m_farPlanesW;

	/// PBO buffer that is used to read the data
	Array<BufferHandle, 2> m_pbos;
	Array<Vec2*, 2> m_pbosAddress;
	DArray<Vec2> m_prevMinMaxDepth;

	/// Main shader program
	ShaderResourcePointer m_shader;
	PipelineHandle m_ppline;

	/// Used to check if the camera is changed and we need to update the planes
	const Camera* m_prevCam = nullptr;

	/// Timestamp for the same reason as prevCam
	Timestamp m_planes4UpdateTimestamp = 0;

	Bool m_enableGpuTests = false;

	ANKI_USE_RESULT Error initInternal();
	ANKI_USE_RESULT Error initPbos();

	void testRange(const CollisionShape& cs, Bool nearPlane,
		U iFrom, U iTo, U jFrom, U jTo, TestResult* visible, 
		U& count) const;

	void testFastSphere(const Sphere& s, const Aabb& aabb,
		TestResult* visible, U& count) const;

	void update(U32 threadId, PtrSize threadsCount, 
		Camera& cam, Bool frustumChanged);

	/// Calculate and set a top looking plane
	void calcPlaneY(U i, const Vec4& projParams);

	/// Calculate and set a right looking plane
	void calcPlaneX(U j, const Vec4& projParams);
};
/// @}

} // end namespace anki

#endif
