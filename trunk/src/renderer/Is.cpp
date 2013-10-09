#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/ThreadPool.h"
#include "anki/core/Counters.h"
#include "anki/core/Logger.h"
#include <sstream>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
/// Check if the prev ground light vector is almost equal to the current
static Bool groundVectorsEqual(const Vec3& prev, const Vec3& crnt)
{
	Bool out = true;
	for(U i = 0; i < 3; i++)
	{
		Bool subout = fabs(prev[i] - crnt[i]) < (getEpsilon<F32>() * 10.0);
		out = out && subout;
	}

	return out;
}

//==============================================================================
/// Clamp a value
template<typename T, typename Y>
void clamp(T& in, Y limit)
{
	in = std::min(in, (T)limit);
}

//==============================================================================
// Shader structs and block representations. All positions and directions in
// viewspace
// For documentation see the shaders

namespace shader {

struct Light
{
	Vec4 posRadius;
	Vec4 diffuseColorShadowmapId;
	Vec4 specularColorTexId;
};

struct PointLight: Light
{};

struct SpotLight: Light
{
	Vec4 lightDir;
	Vec4 outerCosInnerCos;
	Array<Vec4, 4> extendPoints; 
};

struct SpotTexLight: SpotLight
{
	Mat4 texProjectionMat; ///< Texture projection matrix
};

struct CommonUniforms
{
	Vec4 planes;
	Vec4 sceneAmbientColor;
	Vec4 groundLightDir;
};

} // end namespace shader

//==============================================================================
// Use compute shaders on GL >= 4.3
static Bool useCompute()
{
	return GlStateCommonSingleton::get().isComputeShaderSupported()
		&& false;
}

//==============================================================================
/// Write the lights to a client buffer
struct WriteLightsJob: ThreadJob
{
	shader::PointLight* pointLights = nullptr;
	shader::SpotLight* spotLights = nullptr;
	shader::SpotTexLight* spotTexLights = nullptr;

	U8* tileBuffer = nullptr;

	VisibilityTestResults::Container::const_iterator lightsBegin;
	VisibilityTestResults::Container::const_iterator lightsEnd;

	std::atomic<U32>* pointLightsCount = nullptr;
	std::atomic<U32>* spotLightsCount = nullptr;
	std::atomic<U32>* spotTexLightsCount = nullptr;
		
	Array2d<std::atomic<U32>, 
		ANKI_RENDERER_MAX_TILES_Y, 
		ANKI_RENDERER_MAX_TILES_X>
		* tilePointLightsCount = nullptr,
		* tileSpotLightsCount = nullptr,
		* tileSpotTexLightsCount = nullptr;

	Tiler* tiler = nullptr;
	Is* is = nullptr;

	/// Bin lights on CPU path
	Bool binLights = true;

	void operator()(U threadId, U threadsCount)
	{
		U ligthsCount = lightsEnd - lightsBegin;

		// Count job bounds
		U64 start, end;
		choseStartEnd(threadId, threadsCount, ligthsCount, start, end);

		// Run all lights
		for(U64 i = start; i < end; i++)
		{
			const SceneNode* snode = (*(lightsBegin + i)).node;
			const Light* light = snode->getLight();
			ANKI_ASSERT(light);

			switch(light->getLightType())
			{
			case Light::LT_POINT:
				{
					const PointLight& l = 
						*static_cast<const PointLight*>(light);
					I pos = doLight(l);
					if(binLights && pos != -1)
					{
						binLight(l, pos);
					}
				}
				break;
			case Light::LT_SPOT:
				{
					const SpotLight& l = 
						*static_cast<const SpotLight*>(light);
					I pos = doLight(l);
					if(binLights && pos != -1)
					{
						binLight(l, pos);
					}
				}
				break;
			default:
				ANKI_ASSERT(0);
				break;
			}
		}
	}

	/// Copy CPU light to GPU buffer
	I doLight(const PointLight& light)
	{
		// Get GPU light
		I i = pointLightsCount->fetch_add(1);
		if(i >= (I)is->maxPointLights)
		{
			return -1;
		}

		shader::PointLight& slight = pointLights[i];

		const Camera* cam = is->cam;
		ANKI_ASSERT(cam);
	
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
			cam->getViewMatrix());

		slight.posRadius = Vec4(pos, -1.0 / light.getRadius());
		slight.diffuseColorShadowmapId = light.getDiffuseColor();
		slight.specularColorTexId = light.getSpecularColor();

		return i;
	}

	/// Copy CPU spot light to GPU buffer
	I doLight(const SpotLight& light)
	{
		const Camera* cam = is->cam;
		Bool isTexLight = light.getShadowEnabled();
		I i;
		shader::SpotLight* baseslight = nullptr;

		if(isTexLight)
		{
			// Spot tex light

			i = spotTexLightsCount->fetch_add(1);
			if(i >= (I)is->maxSpotTexLights)
			{
				return -1;
			}

			shader::SpotTexLight& slight = spotTexLights[i];
			baseslight = &slight;

			// Write matrix
			static const Mat4 biasMat4(
				0.5, 0.0, 0.0, 0.5, 
				0.0, 0.5, 0.0, 0.5, 
				0.0, 0.0, 0.5, 0.5, 
				0.0, 0.0, 0.0, 1.0);
			// bias * proj_l * view_l * world_c
			slight.texProjectionMat = biasMat4 * light.getProjectionMatrix() *
				Mat4::combineTransformations(light.getViewMatrix(),
				Mat4(cam->getWorldTransform()));

			// Transpose because of driver bug
			slight.texProjectionMat.transpose();
		}
		else
		{
			// Spot light without texture

			i = spotLightsCount->fetch_add(1);
			if(i >= (I)is->maxSpotLights)
			{
				return -1;
			}

			shader::SpotLight& slight = spotLights[i];
			baseslight = &slight;
		}

		// Write common stuff
		ANKI_ASSERT(baseslight);

		// Pos & dist
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam->getViewMatrix());
		baseslight->posRadius = Vec4(pos, -1.0 / light.getDistance());

		// Diff color and shadowmap ID now
		baseslight->diffuseColorShadowmapId = 
			Vec4(light.getDiffuseColor().xyz(), (F32)light.getShadowMapIndex());

		// Spec color
		baseslight->specularColorTexId = light.getSpecularColor();

		// Light dir
		Vec3 lightDir = -light.getWorldTransform().getRotation().getZAxis();
		lightDir = cam->getViewMatrix().getRotationPart() * lightDir;
		baseslight->lightDir = Vec4(lightDir, 0.0);
		
		// Angles
		baseslight->outerCosInnerCos = Vec4(
			light.getOuterAngleCos(),
			light.getInnerAngleCos(), 
			1.0, 
			1.0);

		// extend points
		const PerspectiveFrustum& frustum = light.getFrustum();

		for(U i = 0; i < 4; i++)
		{
			Vec3 extendPoint = light.getWorldTransform().getOrigin() 
				+ frustum.getDirections()[i];

			extendPoint.transform(cam->getViewMatrix());
			baseslight->extendPoints[i] = Vec4(extendPoint, 1.0);
		}

		return i;
	}

	// Bin point light
	void binLight(const PointLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		tiler->test(light.getSpatialCollisionShape(), true, &bitset);

		// Bin to the correct tiles
		PtrSize tilesCount = 
			is->r->getTilesCount().x() * is->r->getTilesCount().y();
		for(U t = 0; t < tilesCount; t++)
		{
			// If not in tile bye
			if(!bitset.test(t))
			{
				continue;
			}

			U x = t % is->r->getTilesCount().x();
			U y = t / is->r->getTilesCount().x();

			U tilePos = (*tilePointLightsCount)[y][x].fetch_add(1);

			if(tilePos < is->maxPointLightsPerTile)
			{
				writeIndexToTileBuffer(0, pos, tilePos, x, y);
			}
		}
	}

	// Bin spot light
	void binLight(const SpotLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		tiler->test(light.getSpatialCollisionShape(), true, &bitset);

		// Bin to the correct tiles
		PtrSize tilesCount = 
			is->r->getTilesCount().x() * is->r->getTilesCount().y();
		for(U t = 0; t < tilesCount; t++)
		{
			// If not in tile bye
			if(!bitset.test(t))
			{
				continue;
			}

			U x = t % is->r->getTilesCount().x();
			U y = t / is->r->getTilesCount().x();

			if(light.getShadowEnabled())
			{
				U tilePos = (*tileSpotTexLightsCount)[y][x].fetch_add(1);

				if(tilePos < is->maxSpotTexLightsPerTile)
				{
					writeIndexToTileBuffer(2, pos, tilePos, x, y);
				}
			}
			else
			{
				U tilePos = (*tileSpotLightsCount)[y][x].fetch_add(1);

				if(tilePos < is->maxSpotLightsPerTile)
				{
					writeIndexToTileBuffer(1, pos, tilePos, x, y);
				}
			}
		}
	}

	/// XXX
	void writeIndexToTileBuffer(
		U lightType, U lightIndex, U indexInTileArray, U tileX, U tileY)
	{
		const PtrSize tileSize = is->calcTileSize();
		PtrSize offset;

		// Calc the start of the tile
		offset = (tileY * is->r->getTilesCount().x() + tileX) * tileSize;

		// Skip the lightsCount header
		offset += sizeof(Vec4);

		// Move to the correct light section
		switch(lightType)
		{
		case 0:
			break;
		case 1:
			offset += sizeof(U32) * is->maxPointLightsPerTile;
			break;
		case 2:
			offset += sizeof(U32) * is->maxPointLightsPerTile
				+ sizeof(U32) * is->maxSpotLightsPerTile;
			break;
		default:
			ANKI_ASSERT(0);
		}

		// Move to the array offset
		offset += sizeof(U32) * indexInTileArray;

		// Write the lightIndex
		*((U32*)(tileBuffer + offset)) = lightIndex;
	}
};

//==============================================================================
Is::Is(Renderer* r_)
	: RenderingPass(r_), sm(r_)
{}

//==============================================================================
Is::~Is()
{}

//==============================================================================
void Is::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init IS") << e;
	}
}

//==============================================================================
void Is::initInternal(const RendererInitializer& initializer)
{
	groundLightEnabled = initializer.get("is.groundLightEnabled");
	maxPointLights = initializer.get("is.maxPointLights");
	maxSpotLights = initializer.get("is.maxSpotLights");
	maxSpotTexLights = initializer.get("is.maxSpotTexLights");

	if(maxPointLights < 1 || maxSpotLights < 1 || maxSpotTexLights < 1)
	{
		throw ANKI_EXCEPTION("Incorrect number of max lights");
	}

	maxPointLightsPerTile = initializer.get("is.maxPointLightsPerTile");
	maxSpotLightsPerTile = initializer.get("is.maxSpotLightsPerTile");
	maxSpotTexLightsPerTile = initializer.get("is.maxSpotTexLightsPerTile");

	if(maxPointLightsPerTile < 1 || maxSpotLightsPerTile < 1 
		|| maxSpotTexLightsPerTile < 1
		|| maxPointLightsPerTile % 4 != 0 || maxSpotLightsPerTile % 4 != 0
		|| maxSpotTexLightsPerTile % 4 != 0)
	{
		throw ANKI_EXCEPTION("Incorrect number of max lights");
	}

	//
	// Init the passes
	//
	sm.init(initializer);

	//
	// Load the programs
	//
	std::stringstream pps;

	pps << "\n#define TILES_X_COUNT " << r->getTilesCount().x()
		<< "\n#define TILES_Y_COUNT " << r->getTilesCount().y()
		<< "\n#define TILES_COUNT " 
		<< (r->getTilesCount().x() * r->getTilesCount().y())
		<< "\n#define RENDERER_WIDTH " << r->getWidth()
		<< "\n#define RENDERER_HEIGHT " << r->getHeight()
		<< "\n#define MAX_POINT_LIGHTS_PER_TILE " << (U32)maxPointLightsPerTile
		<< "\n#define MAX_SPOT_LIGHTS_PER_TILE " << (U32)maxSpotLightsPerTile
		<< "\n#define MAX_SPOT_TEX_LIGHTS_PER_TILE " 
		<< (U32)maxSpotTexLightsPerTile
		<< "\n#define MAX_POINT_LIGHTS " << (U32)maxPointLights
		<< "\n#define MAX_SPOT_LIGHTS " << (U32)maxSpotLights 
		<< "\n#define MAX_SPOT_TEX_LIGHTS " << (U32)maxSpotTexLights
		<< "\n#define GROUND_LIGHT " << groundLightEnabled
		<< "\n#define USE_MRT " << r->getUseMrt()
		<< "\n#define TILES_BLOCK_BINDING " << TILES_BLOCK_BINDING
		<< "\n";

	if(sm.getPcfEnabled())
	{
		pps << "#define PCF 1\n";
	}
	else
	{
		pps << "#define PCF 0\n";
	}

	// point light
	lightPassProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLp.glsl", pps.str().c_str(), "r_").c_str());

#if ANKI_GL == ANKI_GL_DESKTOP
	if(useCompute())
	{
		pps << "#define DEPTHMAP_WIDTH " 
			<< (r->getMs().getDepthFai().getWidth()) << "\n"
			<< "#define DEPTHMAP_HEIGHT " 
			<< (r->getMs().getDepthFai().getHeight()) << "\n"
			<< "#define TILES_BLOCK_BINDING " 
			<< TILES_BLOCK_BINDING << "\n";

		rejectProg.load(ShaderProgramResource::createSrcCodeToCache(
			"shaders/IsRejectLights.glsl", pps.str().c_str(), "r_").c_str());
	}
#endif

	//
	// Create FBOs
	//

	// IS FBO
	fai.create2dFai(r->getWidth(), r->getHeight(), GL_RGB8,
		GL_RGB, GL_UNSIGNED_BYTE);
	fbo.create();
	fbo.setColorAttachments({&fai});

	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}

	//
	// Init the quad
	//
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0},
		{1.0, 0.0}, {0.0, 0.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(
		&quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, 0);

	//
	// Create UBOs
	//

	// Common UBO
	commonUbo.create(sizeof(shader::CommonUniforms), nullptr);

	// lights UBO
	lightsUbo.create(calcLightsUboSize(), nullptr);
	uboAlignment = BufferObject::getUniformBufferOffsetAlignment();

	// tiles BO
	tilesBuffer.create(
		GL_UNIFORM_BUFFER, 
		calcTilesUboSize(), 
		nullptr, 
		GL_DYNAMIC_DRAW);

	ANKI_LOGI("Creating BOs: lights: %uB, tiles: %uB", calcLightsUboSize(), 
		calcTilesUboSize());

	// Sanity checks
	const ShaderProgramUniformBlock* ublock;

	ublock = &lightPassProg->findUniformBlock("commonBlock");
	ublock->setBinding(COMMON_UNIFORMS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::CommonUniforms)
		|| ublock->getBinding() != COMMON_UNIFORMS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the commonBlock");
	}

#if ANKI_GL == ANKI_GL_DESKTOP
	if(rejectProg.isLoaded())
	{
		ublock = &rejectProg->findUniformBlock("commonBlock");
		ublock->setBinding(COMMON_UNIFORMS_BLOCK_BINDING);
		if(ublock->getSize() != sizeof(shader::CommonUniforms)
			|| ublock->getBinding() != COMMON_UNIFORMS_BLOCK_BINDING)
		{
			throw ANKI_EXCEPTION("Problem with the commonBlock");
		}
	}
#endif

	ublock = &lightPassProg->findUniformBlock("pointLightsBlock");
	ublock->setBinding(POINT_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::PointLight) * maxPointLights
		|| ublock->getBinding() != POINT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the pointLightsBlock");
	}

#if ANKI_GL == ANKI_GL_DESKTOP
	if(rejectProg.isLoaded())
	{
		ublock = &rejectProg->findUniformBlock("pointLightsBlock");
		ublock->setBinding(POINT_LIGHTS_BLOCK_BINDING);
		if(ublock->getSize() != sizeof(shader::PointLight) * maxPointLights
			|| ublock->getBinding() != POINT_LIGHTS_BLOCK_BINDING)
		{
			throw ANKI_EXCEPTION("Problem with the pointLightsBlock");
		}
	}
#endif

	ublock = &lightPassProg->findUniformBlock("spotLightsBlock");
	ublock->setBinding(SPOT_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::SpotLight) * maxSpotLights
		|| ublock->getBinding() != SPOT_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("spotTexLightsBlock");
	ublock->setBinding(SPOT_TEX_LIGHTS_BLOCK_BINDING);
	if(ublock->getSize() != sizeof(shader::SpotTexLight) * maxSpotTexLights
		|| ublock->getBinding() != SPOT_TEX_LIGHTS_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the spotTexLightsBlock");
	}

	ublock = &lightPassProg->findUniformBlock("tilesBlock");
	ublock->setBinding(TILES_BLOCK_BINDING);
	if(ublock->getSize() != calcTilesUboSize()
		|| ublock->getBinding() != TILES_BLOCK_BINDING)
	{
		throw ANKI_EXCEPTION("Problem with the tilesBlock");
	}

#if ANKI_GL == ANKI_GL_DESKTOP && 0
	if(rejectProg.isLoaded())
	{
		ublock = &rejectProg->findUniformBlock("tilesBlock");
		ublock->setBinding(TILES_BLOCK_BINDING);
		if(ublock->getSize() != sizeof(shader::Tiles)
			|| ublock->getBinding() != TILES_BLOCK_BINDING)
		{
			throw ANKI_EXCEPTION("Problem with the tilesBlock");
		}
	}
#endif
}

//==============================================================================
void Is::lightPass()
{
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	VisibilityTestResults& vi = 
		cam->getFrustumComponent()->getVisibilityTestResults();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	U visibleSpotTexLightsCount = 0;
	Array<Light*, Sm::MAX_SHADOW_CASTERS> shadowCasters;

	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		Light* light = (*it).node->getLight();
		ANKI_ASSERT(light);
		switch(light->getLightType())
		{
		case Light::LT_POINT:
			// Use % to avoid overflows
			++visiblePointLightsCount;
			break;
		case Light::LT_SPOT:
			{
				SpotLight* slight = static_cast<SpotLight*>(light);
				
				if(slight->getShadowEnabled())
				{
					shadowCasters[visibleSpotTexLightsCount++] = slight;
				}
				else
				{
					++visibleSpotLightsCount;
				}
				break;
			}
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	// Sanitize the counters
	clamp(visiblePointLightsCount, maxPointLights);
	clamp(visibleSpotLightsCount, maxSpotLights);
	clamp(visibleSpotTexLightsCount, maxSpotTexLights);

	ANKI_COUNTER_INC(C_RENDERER_LIGHTS_COUNT, 
		U64(visiblePointLightsCount + visibleSpotLightsCount 
		+ visibleSpotTexLightsCount));

	//
	// Do shadows pass
	//
	sm.run(&shadowCasters[0], visibleSpotTexLightsCount);

	//
	// Write the lights and tiles UBOs
	//

	// Get the offsets and sizes of each uniform block
	PtrSize pointLightsOffset = 0;
	PtrSize pointLightsSize = 
		sizeof(shader::PointLight) * visiblePointLightsCount;
	pointLightsSize = getAlignedRoundUp(uboAlignment, pointLightsSize);

	PtrSize spotLightsOffset = pointLightsSize;
	PtrSize spotLightsSize = 
		sizeof(shader::SpotLight) * visibleSpotLightsCount;
	spotLightsSize = getAlignedRoundUp(uboAlignment, spotLightsSize);

	PtrSize spotTexLightsOffset = spotLightsOffset + spotLightsSize;
	PtrSize spotTexLightsSize = 
		sizeof(shader::SpotTexLight) * visibleSpotTexLightsCount;
	spotTexLightsSize = getAlignedRoundUp(uboAlignment, spotTexLightsSize);

	ANKI_ASSERT(spotTexLightsOffset + spotTexLightsSize <= calcLightsUboSize());

	// Fire the super jobs
	Array<WriteLightsJob, ThreadPool::MAX_THREADS> jobs;

	U8 clientBuffer[32 * 1024]; // Aproximate size
	ANKI_ASSERT(sizeof(clientBuffer) >= calcLightsUboSize());

	U8 tilesClientBuffer[64 * 1024]; // Aproximate size
	ANKI_ASSERT(sizeof(tilesClientBuffer) >= calcTilesUboSize());

	std::atomic<U32> pointLightsAtomicCount(0);
	std::atomic<U32> spotLightsAtomicCount(0);
	std::atomic<U32> spotTexLightsAtomicCount(0);

	Array2d<std::atomic<U32>, 
		ANKI_RENDERER_MAX_TILES_Y, 
		ANKI_RENDERER_MAX_TILES_X> 
		tilePointLightsCount,
		tileSpotLightsCount,
		tileSpotTexLightsCount;

	for(U y = 0; y < r->getTilesCount().y(); y++)
	{
		for(U x = 0; x < r->getTilesCount().x(); x++)
		{
			tilePointLightsCount[y][x].store(0);
			tileSpotLightsCount[y][x].store(0);
			tileSpotTexLightsCount[y][x].store(0);
		}
	}

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		WriteLightsJob& job = jobs[i];

		job.pointLights = 
			(shader::PointLight*)(&clientBuffer[0] + pointLightsOffset);
		job.spotLights = 
			(shader::SpotLight*)(&clientBuffer[0] + spotLightsOffset);
		job.spotTexLights = 
			(shader::SpotTexLight*)(&clientBuffer[0] + spotTexLightsOffset);

		job.tileBuffer = &tilesClientBuffer[0];

		job.lightsBegin = vi.getLightsBegin();
		job.lightsEnd = vi.getLightsEnd();

		job.pointLightsCount = &pointLightsAtomicCount;
		job.spotLightsCount = &spotLightsAtomicCount;
		job.spotTexLightsCount = &spotTexLightsAtomicCount;

		job.tilePointLightsCount = &tilePointLightsCount;
		job.tileSpotLightsCount = &tileSpotLightsCount;
		job.tileSpotTexLightsCount = &tileSpotTexLightsCount;

		job.tiler = &r->getTiler();
		job.is = this;

		threadPool.assignNewJob(i, &job);
	}

	// In the meantime set the state
	setState();

	// Sync
	threadPool.waitForAllJobsToFinish();

	// Write the light count for each tile
	for(U y = 0; y < r->getTilesCount().y(); y++)
	{
		for(U x = 0; x < r->getTilesCount().x(); x++)
		{
			const PtrSize tileSize = calcTileSize();
			UVec4* vec;

			vec = (UVec4*)(
				&tilesClientBuffer[0] + (y * r->getTilesCount().x() + x) 
				* tileSize);

			vec->x() = tilePointLightsCount[y][x].load();
			clamp(vec->x(), maxPointLightsPerTile);

			vec->y() = 0;

			vec->z() = tileSpotLightsCount[y][x].load();
			clamp(vec->z(), maxSpotLightsPerTile);

			vec->w() = tileSpotTexLightsCount[y][x].load();
			clamp(vec->w(), maxSpotTexLightsPerTile);
		}
	}

	// Write BOs
	lightsUbo.write(
		&clientBuffer[0], 0, spotTexLightsOffset + spotTexLightsSize);
	tilesBuffer.write(&tilesClientBuffer[0]);

	//
	// Reject occluded lights. This operation issues a compute job to reject 
	// lights from the tiles
	//
#if ANKI_GL == ANKI_GL_DESKTOP
	if(ANKI_UNLIKELY(rejectProg.isLoaded()))
	{
		rejectProg->bind();

		if(pointLightsSize > 0)
		{
			lightsUbo.setBindingRange(
				POINT_LIGHTS_BLOCK_BINDING, pointLightsOffset, pointLightsSize);
		}
		/*if(spotLightsSize > 0)
		{
			lightsUbo.setBindingRange(SPOT_LIGHTS_BLOCK_BINDING, 
				spotLightsOffset, spotLightsSize);
		}
		if(spotTexLightsSize > 0)
		{
			lightsUbo.setBindingRange(SPOT_TEX_LIGHTS_BLOCK_BINDING, 
				spotTexLightsOffset, spotTexLightsSize);
		}*/
		tilesBuffer.setTarget(GL_SHADER_STORAGE_BUFFER);
		tilesBuffer.setBinding(TILES_BLOCK_BINDING);

		commonUbo.setBinding(COMMON_UNIFORMS_BLOCK_BINDING);

		rejectProg->findUniformVariable("depthMap").set(
			r->getMs().getDepthFai());

		glDispatchCompute(r->getTilesCount().x(), r->getTilesCount().y(), 1);

		tilesBuffer.setTarget(GL_UNIFORM_BUFFER);
	}
#endif	

	//
	// Setup shader and draw
	//

	// shader prog
	lightPassProg->bind();

	commonUbo.setBinding(COMMON_UNIFORMS_BLOCK_BINDING);

	lightPassProg->findUniformVariable("limitsOfNearPlane").set(
		Vec4(r->getLimitsOfNearPlane(), r->getLimitsOfNearPlane2()));

	if(pointLightsSize > 0)
	{
		lightsUbo.setBindingRange(POINT_LIGHTS_BLOCK_BINDING, pointLightsOffset,
			pointLightsSize);
	}
	if(spotLightsSize > 0)
	{
		lightsUbo.setBindingRange(SPOT_LIGHTS_BLOCK_BINDING, spotLightsOffset,
			spotLightsSize);
	}
	if(spotTexLightsSize > 0)
	{
		lightsUbo.setBindingRange(SPOT_TEX_LIGHTS_BLOCK_BINDING, 
			spotTexLightsOffset, spotTexLightsSize);
	}
	tilesBuffer.setBinding(TILES_BLOCK_BINDING);

	lightPassProg->findUniformVariable("msFai0").set(r->getMs().getFai0());
	if(r->getUseMrt())
	{
		lightPassProg->findUniformVariable("msFai1").set(r->getMs().getFai1());
	}
	lightPassProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());
	lightPassProg->findUniformVariable("shadowMapArr").set(sm.sm2DArrayTex);

	quadVao.bind();
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 
		r->getTilesCount().x() * r->getTilesCount().y());
}

//==============================================================================
void Is::setState()
{
	Bool drawToDefaultFbo = !r->getPps().getEnabled() 
		&& !r->getDbg().getEnabled() 
		&& !r->getIsOffscreen()
		&& r->getRenderingQuality() == 1.0;

	if(drawToDefaultFbo)
	{
		Fbo::bindDefault(Fbo::FT_ALL, true);

		GlStateSingleton::get().setViewport(
			0, 0, r->getWindowWidth(), r->getWindowHeight());
	}
	else
	{
		fbo.bind(Fbo::FT_ALL, true);

		GlStateSingleton::get().setViewport(
			0, 0, r->getWidth(), r->getHeight());
	}

	GlStateSingleton::get().disable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);
}

//==============================================================================
void Is::run()
{
	SceneGraph& scene = r->getSceneGraph();
	cam = &scene.getActiveCamera();

	GlStateSingleton::get().disable(GL_BLEND);

	// Ground light direction
	Vec3 groundLightDir;
	if(groundLightEnabled)
	{
		groundLightDir = -cam->getViewMatrix().getColumn(1).xyz();
	}

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp < scene.getAmbientColorUpdateTimestamp()
		|| commonUboUpdateTimestamp == 1
		|| (groundLightEnabled
			&& !groundVectorsEqual(groundLightDir, prevGroundLightDir)))
	{
		shader::CommonUniforms blk;
		blk.planes = Vec4(r->getPlanes().x(), r->getPlanes().y(), 0.0, 0.0);
		blk.sceneAmbientColor = scene.getAmbientColor();

		if(groundLightEnabled)
		{
			blk.groundLightDir = Vec4(groundLightDir, 1.0);
			prevGroundLightDir = groundLightDir;
		}

		commonUbo.write(&blk);

		commonUboUpdateTimestamp = getGlobTimestamp();
	}

	// Do the light pass including the shadow passes
	lightPass();
}

//==============================================================================
PtrSize Is::calcLightsUboSize() const
{
	PtrSize size;
	PtrSize uboAlignment = BufferObject::getUniformBufferOffsetAlignment();

	size = getAlignedRoundUp(
		uboAlignment,
		maxPointLights * sizeof(shader::PointLight));

	size += getAlignedRoundUp(
		uboAlignment,
		maxSpotLights * sizeof(shader::SpotLight));

	size += getAlignedRoundUp(
		uboAlignment,
		maxSpotTexLights * sizeof(shader::SpotTexLight));

	return size;
}

//==============================================================================
PtrSize Is::calcTileSize() const
{
	PtrSize size =
		sizeof(Vec4) // lightsCount
		+ maxPointLightsPerTile * sizeof(U32) // pointLightIndices
		+ maxSpotLightsPerTile * sizeof(U32) // spotLightIndices
		+ maxSpotTexLightsPerTile * sizeof(U32); // spotTexLightIndices

	return size;
}

//==============================================================================
PtrSize Is::calcTilesUboSize() const
{
	return calcTileSize() * r->getTilesCount().x() * r->getTilesCount().y();
}

} // end namespace anki
