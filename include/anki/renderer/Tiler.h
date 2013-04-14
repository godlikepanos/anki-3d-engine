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
	typedef std::bitset<ANKI_RENDERER_TILES_COUNT> Bitset;

	Tiler();
	~Tiler();

	void init(Renderer* r);

	/// Issue the GPU job
	void runMinMax(const Texture& depthMap);

	/// Update the tiles before doing visibility tests
	void updateTiles(Camera& cam);

	/// Test against all tiles
	Bool test(
		const CollisionShape& cs,
		Bool nearPlane,
		Bitset* mask) const;

private:
	/// Tile planes
	Vector<Plane> allPlanes;
	Plane* planesI = nullptr;
	Plane* planesJ = nullptr;
	Plane* planesIW = nullptr;
	Plane* planesJW = nullptr;
	Plane* nearPlanesW = nullptr;
	Plane* farPlanesW = nullptr;

	/// The timestamp of the 4 planes update
	U32 planes4UpdateTimestamp = Timestamp::getTimestamp();

	/// A texture of TILES_X_COUNT*TILES_Y_COUNT size and format XXX. Used to
	/// calculate the near and far planes of the tiles
	Texture fai;

	/// Main FBO
	Fbo fbo;

	/// PBO
	Pbo pbo;

	/// Main shader program
	ShaderProgramResourcePointer prog;

	const ShaderProgramUniformVariable* depthMapUniform = nullptr; ///< Cache it

	Renderer* r = nullptr;
	const Camera* prevCam = nullptr;

	void initInternal(Renderer* r_);

	void testRange(const CollisionShape& cs, Bool nearPlane,
		U iFrom, U iTo, U jFrom, U jTo, Bitset& bitset) const;
};

} // end namespace anki

#endif
