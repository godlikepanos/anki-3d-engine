#ifndef ANKI_RENDERER_TILER_H
#define ANKI_RENDERER_TILER_H

#include "anki/util/StdTypes.h"
#include "anki/collision/Collision.h"
#include "anki/gl/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"

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

public:
	// Config. These values affect the size of the uniform blocks and keep in
	// mind that there are size limitations in uniform blocks.
	static const U TILES_X_COUNT = 16;
	static const U TILES_Y_COUNT = 16;

	Tiler();
	~Tiler();

	void init(Renderer* r);

	/// Issue the GPU job
	void runMinMax(const Texture& depthMap);

	/// Update the tiles before doing visibility tests
	void updateTiles(Camera& cam);

	/// Test against all tiles
	Bool testAll(const CollisionShape& cs,
 		const Bool skipNearPlaneCheck = false) const;

	/// Test against all tiles and return affected tiles
	Bool testAll(const CollisionShape& cs,
 		U32* tileIds, U32& tilesCount, 
		const Bool skipNearPlaneCheck = false) const;
 
	/// Test on a specific tile
	Bool test(const CollisionShape& cs, 
		const U32 tileId, const Bool skipNearPlaneCheck = false) const;

	Bool test(const Spatial& sp, const Frustumable& fr) const;

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

	/// XXX
	struct Tile_
	{
		Vec3 min;
		Vec3 max;
		Array<U32, 2> mask;
		Array<I16, 4> children; ///< Use small index to save memory
	};

	typedef F32 PixelArray[TILES_Y_COUNT][TILES_X_COUNT][2];

	/// @note The [0][0] is the bottom left tile
	union
	{
		Array<Array<Tile, TILES_X_COUNT>, TILES_Y_COUNT> tiles;
		Array<Tile, TILES_X_COUNT * TILES_Y_COUNT> tiles1d;
	};

	Vector<Tile_> tiles_;
	Tile_* tiles0; ///< Tiles last level

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

	const ShaderProgramUniformVariable* depthMapUniform; ///< Cache it

	Renderer* r;
	const Camera* prevCam;

	void initInternal(Renderer* r);
	Tile_* initTilesInDepth(Tile_* tiles, U depth);
	void initTiles();

	void updateTilesInternal();
	Vec2 updateTileMinMax(Tile_& tile); ///< Recursive 

	Bool testInternal(const CollisionShape& cs, const Tile& tile, 
		const U startPlane) const;
	void testTile(const Tile_& tile, const Vec2& a, const Vec2& b, 
		const Vec2& objectMinMaxZ,
		Array<U32, 2>& mask) const;
};

} // end namespace anki

#endif
