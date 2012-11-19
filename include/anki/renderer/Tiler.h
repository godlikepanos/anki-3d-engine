#ifndef ANKI_RENDERER_TILER_H
#define ANKI_RENDERER_TILER_H

#include "anki/util/StdTypes.h"
#include "anki/collision/Collision.h"
#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"

namespace anki {

class Renderer;
class Camera;

/// Tiler used for visibility tests
class Tiler
{
	friend struct UpdateTiles2PlanesJob;
	friend struct UpdateTiles4PlanesPerspectiveCameraJob;

public:
	// Config. These values affect the size of the uniform blocks and keep in
	// mind that there are size limitations in uniform blocks.
	static const U TILES_X_COUNT = 16;
	static const U TILES_Y_COUNT = 16;

	Tiler();
	~Tiler();

	void init(Renderer* r);

	/// Update the tiles before doing visibility tests
	void updateTiles(Camera& cam, const Texture& depthMap);

	/// Test against all tiles
	Bool testAll(const CollisionShape& cs,
 		const Bool skipNearPlaneCheck = false) const;
 
	/// Test on a specific tile
	Bool test(const CollisionShape& cs, 
		const U32 tileId, const Bool skipNearPlaneCheck = false) const;

private:
	/// A screen tile
	struct Tile
	{
		/// @name Frustum planes
		/// @{
		Array<Plane, Frustum::FP_COUNT> planes; ///< In local space
		Array<Plane, Frustum::FP_COUNT> planesWSpace; ///< In world space
		/// @}
	};

	typedef F32 PixelArray[TILES_Y_COUNT][TILES_X_COUNT][2];

	/// @note The [0][0] is the bottom left tile
	union
	{
		Array<Array<Tile, TILES_X_COUNT>, TILES_Y_COUNT> tiles;
		Array<Tile, TILES_X_COUNT * TILES_Y_COUNT> tiles1d;
	};

	/// The timestamp of the 4 planes update
	U32 planes4UpdateTimestamp = Timestamp::getTimestamp();

	/// A texture of TILES_X_COUNT*TILES_Y_COUNT size and format XXX. Used to
	/// calculate the near and far planes of the tiles
	Texture fai;

	/// Main FBO
	Fbo fbo;

	/// Main shader program
	ShaderProgramResourcePointer prog;

	Renderer* r;
	const Camera* prevCam;

	void initInternal(Renderer* r);

	void update4Planes(Camera& cam, void* jobs);
	void waitUpdate4Planes();
	void update2Planes(Camera& cam, const PixelArray& pixels);

	Bool testInternal(const CollisionShape& cs, const Tile& tile, 
		const U startPlane) const;
};

} // end namespace anki

#endif
