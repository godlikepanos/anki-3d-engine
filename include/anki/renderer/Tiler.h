// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
struct VisibleTiles
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
};

/// Tiler used for visibility tests
class Tiler: public RenderingPass
{
	friend struct UpdatePlanesPerspectiveCameraTask;

public:
	Tiler(Renderer* r);
	~Tiler();

	Error init();

	/// Issue the GPU job
	void runMinMax(const GlTextureHandle& depthMap);

	/// Update the tiles before doing visibility tests
	void updateTiles(Camera& cam);

	/// Test against all tiles.
	/// @param[in]  collisionShape The collision shape to test.
	/// @param      nearPlane      If true check against the near plane as well.
	/// @param[out] visible        A list with the tiles that contain the 
	///                            collision shape.
	Bool test(
		const CollisionShape& collisionShape,
		Bool nearPlane,
		VisibleTiles* visible) const;

private:
	/// Tile planes
	DArray<Plane> m_allPlanes;
	Plane* m_planesY = nullptr;
	Plane* m_planesX = nullptr;
	Plane* m_planesYW = nullptr;
	Plane* m_planesXW = nullptr;
	Plane* m_nearPlanesW = nullptr;
	Plane* m_farPlanesW = nullptr;

	/// A texture of tilesXCount * tilesYCount size and format RG32UI. Used to
	/// calculate the near and far planes of the tiles
	GlTextureHandle m_rt;

	/// Main FB for the fai
	GlFramebufferHandle m_fb;

	/// PBO buffer that is used to read the data of fai asynchronously
	GlBufferHandle m_pixelBuff;

	/// Main shader program
	ProgramResourcePointer m_frag;
	GlPipelineHandle m_ppline;

	/// Used to check if the camera is changed and we need to update the planes
	const Camera* m_prevCam = nullptr;

	/// Timestamp for the same reason as prevCam
	Timestamp m_planes4UpdateTimestamp = getGlobTimestamp();

	ANKI_USE_RESULT Error initInternal();

	void testRange(const CollisionShape& cs, Bool nearPlane,
		U iFrom, U iTo, U jFrom, U jTo, VisibleTiles* visible, 
		U& count) const;

	void update(U32 threadId, PtrSize threadsCount, 
		Camera& cam, Bool frustumChanged);

	/// Calculate and set a top looking plane
	void calcPlaneY(U i, const F32 o6, const F32 near) const;

	/// Calculate and set a right looking plane
	void calcPlaneX(U j, const F32 l6, const F32 near) const;
};

/// @}

} // end namespace anki

#endif
