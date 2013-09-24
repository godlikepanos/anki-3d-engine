#ifndef ANKI_RENDERER_TILER_H
#define ANKI_RENDERER_TILER_H

#include "anki/util/StdTypes.h"
#include "anki/Collision.h"
#include "anki/gl/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"
#include <bitset>

namespace anki {

class Renderer;
class Camera;
class ShaderProgramUniformVariable;
class Spatial;
class Frustumable;

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
	void runMinMax(const Texture& depthMap);

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
	Vector<Plane> allPlanes;
	Plane* planesY = nullptr;
	Plane* planesX = nullptr;
	Plane* planesYW = nullptr;
	Plane* planesXW = nullptr;
	Plane* nearPlanesW = nullptr;
	Plane* farPlanesW = nullptr;

	/// A texture of tilesXCount * tilesYCount size and format RG32UI. Used to
	/// calculate the near and far planes of the tiles
	Texture fai;

	/// Main FBO for the fai
	Fbo fbo;

	/// PBO buffer that is used to read the data of fai asynchronously
	Pbo pbo;

	/// Main shader program
	ShaderProgramResourcePointer prog;

	const ShaderProgramUniformVariable* depthMapUniform = nullptr; ///< Cache it

	Renderer* r = nullptr;

	/// Used to check if the camera is changed and we need to update the planes
	const Camera* prevCam = nullptr;

	/// Timestamp for the same reason as prevCam
	Timestamp planes4UpdateTimestamp = getGlobTimestamp();

	void initInternal(Renderer* r_);

	void testRange(const CollisionShape& cs, Bool nearPlane,
		U iFrom, U iTo, U jFrom, U jTo, Bitset& bitset) const;
};

} // end namespace anki

#endif
