// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_TILER_H
#define ANKI_RENDERER_TILER_H

#include "anki/util/StdTypes.h"
#include "anki/Collision.h"
#include "anki/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"
#include <bitset>

namespace anki {

class Renderer;
class Camera;
class ShaderProgramUniformVariable;
class Frustumable;

/// @addtogroup renderer
/// @{

/// Tiler used for visibility tests
class Tiler
{
	friend struct UpdateTilesPlanesPerspectiveCameraJob;
	friend struct UpdatePlanesPerspectiveCameraJob;

public:
	typedef std::bitset<
		ANKI_RENDERER_MAX_TILES_X * ANKI_RENDERER_MAX_TILES_Y> Bitset;

	Tiler();
	~Tiler();

	void init(Renderer* r);

	/// Issue the GPU job
	void runMinMax(const GlTextureHandle& depthMap);

	/// Update the tiles before doing visibility tests
	void updateTiles(Camera& cam);

	/// Test against all tiles
	/// @param[in]  collisionShape The collision shape to test
	/// @param      nearPlane      If true check against the near plane as well
	/// @param[out] mask           A bitmask that indicates the tiles that the
	///                            give collision shape is inside
	Bool test(
		const CollisionShape& collisionShape,
		Bool nearPlane,
		Bitset* mask) const;

private:
	/// Tile planes
	Vector<Plane> m_allPlanes;
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
	GlProgramPipelineHandle m_ppline;

	Renderer* m_r = nullptr;

	/// Used to check if the camera is changed and we need to update the planes
	const Camera* m_prevCam = nullptr;

	/// Timestamp for the same reason as prevCam
	Timestamp m_planes4UpdateTimestamp = getGlobTimestamp();

	void initInternal(Renderer* r);

	void testRange(const CollisionShape& cs, Bool nearPlane,
		U iFrom, U iTo, U jFrom, U jTo, Bitset& bitset) const;
};

/// @}

} // end namespace anki

#endif
