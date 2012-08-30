#ifndef ANKI_RENDERER_IS_H
#define ANKI_RENDERER_IS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/gl/Gl.h"
#include "anki/math/Math.h"
#include "anki/renderer/Sm.h"
#include "anki/renderer/Smo.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/core/Timestamp.h"

namespace anki {

class PointLight;
class SpotLight;

/// Illumination stage
class Is: private RenderingPass
{
public:
	static const U TILES_X_COUNT = 16;
	static const U TILES_Y_COUNT = 16;

	static const U MAX_LIGHTS_PER_TILE = 128;

	static const U MAX_LIGHTS = 1024;

	Is(Renderer* r);

	~Is();

	void init(const RendererInitializer& initializer);
	void run();

	/// @name Accessors
	/// @{
	const Texture& getFai() const
	{
		return fai;
	}

	const Texture& getMinMaxFai() const
	{
		return minMaxFai;
	}
	/// @}

private:
	enum LightSubType
	{
		LST_POINT,
		LST_SPOT,
		LST_SPOT_SHADOW,
		LST_COUNT
	};

	/// A screen tile
	struct Tile
	{
		/// depth[0] = min depth, depth[1] = max depth
		Vec2 depth;

		/// Used for 2D light culling
		/// @note The coords are in NDC
		/// @note coords[0] is the bottom left coord, and the coords[1] the 
		///       top right
		Array<Vec2, 2> coords;

		Array<U32, MAX_LIGHTS_PER_TILE> lightIndices;
		U lightsCount = 0;

		/// Frustum planes
		Array<Plane, 6> planes;
	};

	U32 planesUpdateTimestamp = Timestamp::getTimestamp();

	/// @note The [0][0] is the bottom left tile
	Tile tiles[TILES_Y_COUNT][TILES_X_COUNT];

	/// A texture of TILES_X_COUNT*TILES_Y_COUNT size and format RG16F. Used to
	/// to fill the Tile::depth
	Texture minMaxFai;

	/// The IS FAI
	Texture fai;

	/// An FBO to write to the minMaxTex
	Fbo minMaxTilerFbo;

	/// The IS FBO
	Fbo fbo;

	/// Contains common data for all shader programs
	Ubo commonUbo;

	/// Contains the indices of lights per tile
	Ubo lightIndicesUbo;

	/// Contains info of all the lights
	Ubo lightsUbo;

	/// Min max shader program
	ShaderProgramResourcePointer minMaxPassSprog;

	/// Light shaders
	Array<ShaderProgramResourcePointer, LST_COUNT> lightSProgs;

	Sm sm;

	/// Project a sphere to a circle
	static void projectShape(const Camera& cam,
		const Sphere& sphere, Vec2& circleCenter, F32& circleRadius);
	/// Project a perspective frustum to a triangle
	static void projectShape(const Mat4& projectionMat,
		const PerspectiveFrustum& fr, Array<Vec2, 3>& coords);

	/// For intersecting of point lights
	static Bool circleIntersects(const Tile& tile, const Vec2& circleCenter, 
		F32 circleRadius);
	/// For intersecting of spot lights
	static Bool triangleIntersects(const Tile& tile, 
		const Array<Vec2, 3>& coords);

	/// Updates all the planes except the near and far plane. Near and far 
	/// planes will be updated in min max pass when the depth is known
	void updateAllTilesPlanes();

	void updateAllTilesPlanes(const PerspectiveCamera& pcam);

	/// Fill the minMaxFai
	void minMaxPass();

	/// See if the light is inside the tile
	Bool cullLight(const PointLight& light, const Tile& tile);
	Bool cullLight(const SpotLight& light, const Tile& tile);

	/// Do the light culling
	void tileCulling();

	void initInternal(const RendererInitializer& initializer);

	void pointLightsPass();
};

} // end namespace anki

#endif
