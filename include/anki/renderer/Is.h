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
struct ShaderPointLights;
struct ShaderSpotLights;
struct ShaderTiles;

/// Illumination stage
class Is: private RenderingPass
{
	friend struct WritePointLightsUbo;
	friend struct WriteSpotLightsUbo;
	friend struct WriteTilesUboJob;

public:
	// Config. These values affect the size of the uniform blocks and keep in
	// mind that there are size limitations in uniform blocks.
	static const U TILES_X_COUNT = 16;
	static const U TILES_Y_COUNT = 16;

	static const U MAX_LIGHTS_PER_TILE = 32;

	static const U MAX_POINT_LIGHTS = 512;
	static const U MAX_SPOT_LIGHTS = 8;

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
		Array<U32, MAX_LIGHTS_PER_TILE> lightIndices;
		U lightsCount = 0;

		/// Frustum planes
		Array<Plane, 6> planes;
	};

	static const U COMMON_UNIFORMS_BLOCK_BINDING = 0;
	static const U POINT_LIGHTS_BLOCK_BINDING = 1;
	static const U SPOT_LIGHTS_BLOCK_BINDING = 2;
	static const U TILES_BLOCK_BINDING = 3;

	U32 planesUpdateTimestamp = Timestamp::getTimestamp();

	/// @note The [0][0] is the bottom left tile
	Array<Array<Tile, TILES_X_COUNT>, TILES_Y_COUNT> tiles;

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

	/// Track the updates of commonUbo
	U32 commonUboUpdateTimestamp = Timestamp::getTimestamp();

	/// Contains info of for lights
	Ubo pointLightsUbo;
	Ubo spotLightsUbo;

	/// Contains the indices of lights per tile
	Ubo tilesUbo;

	/// Min max shader program
	ShaderProgramResourcePointer minMaxPassSprog;

	/// Light shaders
	ShaderProgramResourcePointer lightPassProg;

	Sm sm;

	void initInternal(const RendererInitializer& initializer);

	/// Updates all the planes except the near and far plane. Near and far 
	/// planes will be updated in min max pass when the depth is known
	void updateAllTilesPlanes();

	void updateAllTilesPlanesInternal(const PerspectiveCamera& pcam);

	/// Do minmax pass and set the near/far planes of the tiles
	void updateTiles();

	/// See if the light is inside the tile
	static Bool cullLight(const PointLight& light, const Tile& tile, 
		const Mat4& viewMatrix);
	static Bool cullLight(const SpotLight& light, const Tile& tile,
		const Mat4& viewMatrix);

	void writeLightUbo(ShaderPointLights& shaderLights, U32 maxShaderLights,
		PointLight** visibleLights, U32 visibleLightsCount, U start, U end);
	void writeLightUbo(ShaderSpotLights& shaderLights, U32 maxShaderLights,
		SpotLight** visibleLights, U32 visibleLightsCount, U start, U end);

	void writeTilesUbo(
		PointLight* visiblePointLights[], U32 visiblePointLightsCount,
		SpotLight* visibleSpotLights[], U32 visibleSpotLightsCount,
		ShaderTiles& shaderTiles, U32 maxLightsPerTile,
		U32 start, U32 end);

	void pointLightsPass();
};

} // end namespace anki

#endif
