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

namespace anki {

class PointLight;
class SpotLight;

/// Illumination stage
class Is: private RenderingPass
{
public:
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
	/// @}

private:
	enum LightSubType
	{
		LST_POINT,
		LST_SPOT,
		LST_SPOT_SHADOW
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
		Vec2 coords[2]; 

		Vector<PointLight*> plights;
		Vector<SpotLight*> slights;
		Vector<SpotLight*> slightsShadow;
		Ubo lightIndicesUbo;
	};

	static const U TILES_X_COUNT = 16;
	static const U TILES_Y_COUNT = 16;

	static const U MAX_LIGHTS_PER_TILE = 128;

	static const U MAX_LIGHTS = 1024;

	/// @note The [0][0] is the bottom left tile
	Tile tiles[TILES_X_COUNT][TILES_Y_COUNT];

	/// A texture of TILES_X_COUNT*TILES_Y_COUNT size and format RG16F. Used to
	/// to fill the Tile::depth
	Texture minMaxFai;

	/// An FBO to write to the minMaxTex
	Fbo minMaxTilerFbo;

	/// Min max shader program
	ShaderProgramResourcePointer minMaxPassSprog;

	/// Project a sphere to a circle
	static void projectShape(const Mat4& projectionMat, 
		const Sphere& sphere, Vec2& circleCenter, F32& circleRadius);
	/// Project a perspective frustum to a triangle
	static void projectShape(const Mat4& projectionMat,
		const PerspectiveFrustum& fr, std::array<Vec2, 3>& coords);

	/// For intersecting of point lights
	static Bool circleIntersects(const Tile& tile, const Vec2& circleCenter, 
		F32 circleRadius);
	/// For intersecting of spot lights
	static Bool triangleIntersects(const Tile& tile, 
		const std::array<Vec2, 3>& coords);

	/// Fill the minMaxFai
	void minMaxPass();

	/// See if the light is inside the tile and if it is assign it to the tile
	void cullLight(Tile& tile, Light& light);

	/// Do the light culling
	void tileCulling();

	void initTiles();

	Smo smo;
	Sm sm;
	Texture fai;
	Fbo fbo;
	BufferObject generalUbo;

	/// Illumination stage ambient pass shader program
	ShaderProgramResourcePointer ambientPassSProg;
	/// Illumination stage point light shader program
	ShaderProgramResourcePointer pointLightSProg;
	/// Illumination stage spot light w/o shadow shader program
	ShaderProgramResourcePointer spotLightNoShadowSProg;
	/// Illumination stage spot light w/ shadow shader program
	ShaderProgramResourcePointer spotLightShadowSProg;

	/// The ambient pass
	void ambientPass(const Vec3& color);

	/// The point light pass
	void pointLightPass(PointLight& plight);

	/// The spot light pass
	void spotLightPass(SpotLight& slight);
};

} // end namespace anki

#endif
