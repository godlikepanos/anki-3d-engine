#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/core/Threadpool.h"
#include "anki/core/Counters.h"
#include "anki/core/Logger.h"
#include <sstream>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

const PtrSize TILE_SIZE = sizeof(UVec4);

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
struct WriteLightsJob: ThreadpoolTask
{
	shader::PointLight* pointLights = nullptr;
	shader::SpotLight* spotLights = nullptr;
	shader::SpotTexLight* spotTexLights = nullptr;

	U8* tileBuffer = nullptr;
	U8* plightsIdsBuffer = nullptr;
	U8* slightsIdsBuffer = nullptr;

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

	void operator()(ThreadId threadId, U threadsCount)
	{
		U ligthsCount = lightsEnd - lightsBegin;

		// Count job bounds
		PtrSize start, end;
		choseStartEnd(threadId, threadsCount, ligthsCount, start, end);

		// Run all lights
		for(U64 i = start; i < end; i++)
		{
			SceneNode* snode = (*(lightsBegin + i)).node;
			Light* light = staticCastPtr<Light*>(snode);
			ANKI_ASSERT(light);

			switch(light->getLightType())
			{
			case Light::LT_POINT:
				{
					PointLight& l = 
						*staticCastPtr<PointLight*>(light);
					I pos = doLight(l);
					if(binLights && pos != -1)
					{
						binLight(l, pos);
					}
				}
				break;
			case Light::LT_SPOT:
				{
					SpotLight& l = *staticCastPtr<SpotLight*>(light);
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
	void binLight(PointLight& light, U pos)
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
	void binLight(SpotLight& light, U pos)
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
		U8* storage = nullptr;
		U tileIndex = tileY * is->r->getTilesCount().x() + tileX;

		// Move to the correct light section
		switch(lightType)
		{
		case 0:
			storage = plightsIdsBuffer 
				+ tileIndex * is->maxPointLightsPerTile * sizeof(U32);
			break;
		case 1:
			storage = slightsIdsBuffer 
				+ tileIndex 
				* (is->maxSpotLightsPerTile + is->maxSpotTexLightsPerTile) 
				* sizeof(U32);
			break;
		case 2:
			storage = slightsIdsBuffer 
				+ tileIndex 
				* (is->maxSpotLightsPerTile + is->maxSpotTexLightsPerTile) 
				* sizeof(U32)
				+ is->maxSpotLightsPerTile * sizeof(U32);
			break;
		default:
			ANKI_ASSERT(0);
		}

		// Move to the array offset
		storage += sizeof(U32) * indexInTileArray;

		// Write the lightIndex
		*((U32*)(storage)) = lightIndex;
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

	if(sm.getPoissonEnabled())
	{
		pps << "#define POISSON 1\n";
	}
	else
	{
		pps << "#define POISSON 0\n";
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

	uboAlignment = BufferObject::getUniformBufferOffsetAlignment();

	commonUbo.create(GL_UNIFORM_BUFFER, sizeof(shader::CommonUniforms), 
		nullptr, GL_DYNAMIC_DRAW);

	lightsUbo.create(GL_UNIFORM_BUFFER, calcLightsUboSize(), nullptr,
		GL_DYNAMIC_DRAW);

	tilesBuffer.create(
		GL_UNIFORM_BUFFER, 
		r->getTilesCount().x() * r->getTilesCount().y() * sizeof(UVec4),
		nullptr, 
		GL_DYNAMIC_DRAW);

	pointLightIndicesBuffer.create(
		GL_UNIFORM_BUFFER, 
		calcPointLightIndicesBufferSize(),
		nullptr, 
		GL_DYNAMIC_DRAW);

	spotLightIndicesBuffer.create(
		GL_UNIFORM_BUFFER, 
		calcSpotLightIndicesBufferSize(),
		nullptr, 
		GL_DYNAMIC_DRAW);

	// Sanity checks
	blockSetupAndSanityCheck("commonBlock", COMMON_UNIFORMS_BLOCK_BINDING,
		commonUbo.getSizeInBytes());

	blockSetupAndSanityCheck("pointLightsBlock", POINT_LIGHTS_BLOCK_BINDING,
		sizeof(shader::PointLight) * maxPointLights);

	blockSetupAndSanityCheck("spotLightsBlock", SPOT_LIGHTS_BLOCK_BINDING,
		sizeof(shader::SpotLight) * maxSpotLights);

	blockSetupAndSanityCheck("spotTexLightsBlock", 
		SPOT_TEX_LIGHTS_BLOCK_BINDING,
		sizeof(shader::SpotTexLight) * maxSpotTexLights);

	blockSetupAndSanityCheck("tilesBlock", TILES_BLOCK_BINDING,
		tilesBuffer.getSizeInBytes());

	blockSetupAndSanityCheck("pointLightIndicesBlock", 
		TILES_POINT_LIGHT_INDICES_BLOCK_BINDING,
		pointLightIndicesBuffer.getSizeInBytes());

	blockSetupAndSanityCheck("spotLightIndicesBlock", 
		TILES_SPOT_LIGHT_INDICES_BLOCK_BINDING,
		spotLightIndicesBuffer.getSizeInBytes());
}

//==============================================================================
void Is::lightPass()
{
	Threadpool& threadPool = ThreadpoolSingleton::get();
	VisibilityTestResults& vi = 
		cam->getVisibilityTestResults();

	SceneFrameAllocator<U8> alloc = r->getSceneGraph().getFrameAllocator();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	U visibleSpotTexLightsCount = 0;
	Array<Light*, Sm::MAX_SHADOW_CASTERS> shadowCasters;

	for(auto it : vi.lights)
	{
		Light* light = staticCastPtr<Light*>(it.node);
		switch(light->getLightType())
		{
		case Light::LT_POINT:
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
	alignRoundUp(uboAlignment, pointLightsSize);

	PtrSize spotLightsOffset = pointLightsSize;
	PtrSize spotLightsSize = 
		sizeof(shader::SpotLight) * visibleSpotLightsCount;
	alignRoundUp(uboAlignment, spotLightsSize);

	PtrSize spotTexLightsOffset = spotLightsOffset + spotLightsSize;
	PtrSize spotTexLightsSize = 
		sizeof(shader::SpotTexLight) * visibleSpotTexLightsCount;
	alignRoundUp(uboAlignment, spotTexLightsSize);

	ANKI_ASSERT(spotTexLightsOffset + spotTexLightsSize <= calcLightsUboSize());

	// Fire the super jobs
	Array<WriteLightsJob, Threadpool::MAX_THREADS> jobs;

	U8* lightsClientBuffer = alloc.allocate(lightsUbo.getSizeInBytes());

	U8* tilesClientBuffer = alloc.allocate(tilesBuffer.getSizeInBytes());

	U8* plightIdsClientBuffer = 
		alloc.allocate(pointLightIndicesBuffer.getSizeInBytes());

	U8* slightIdsClientBuffer = 
		alloc.allocate(spotLightIndicesBuffer.getSizeInBytes());

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
			(shader::PointLight*)(&lightsClientBuffer[0] + pointLightsOffset);
		job.spotLights = 
			(shader::SpotLight*)(&lightsClientBuffer[0] + spotLightsOffset);
		job.spotTexLights = 
			(shader::SpotTexLight*)(
			&lightsClientBuffer[0] + spotTexLightsOffset);

		job.tileBuffer = tilesClientBuffer;
		job.plightsIdsBuffer = plightIdsClientBuffer;
		job.slightsIdsBuffer = slightIdsClientBuffer;

		job.lightsBegin = vi.lights.begin();
		job.lightsEnd = vi.lights.end();

		job.pointLightsCount = &pointLightsAtomicCount;
		job.spotLightsCount = &spotLightsAtomicCount;
		job.spotTexLightsCount = &spotTexLightsAtomicCount;

		job.tilePointLightsCount = &tilePointLightsCount;
		job.tileSpotLightsCount = &tileSpotLightsCount;
		job.tileSpotTexLightsCount = &tileSpotTexLightsCount;

		job.tiler = &r->getTiler();
		job.is = this;

		threadPool.assignNewTask(i, &job);
	}

	// In the meantime set the state
	setState();

	// Sync
	threadPool.waitForAllThreadsToFinish();

	// Write the light count for each tile
	for(U y = 0; y < r->getTilesCount().y(); y++)
	{
		for(U x = 0; x < r->getTilesCount().x(); x++)
		{
			const PtrSize tileSize = TILE_SIZE;
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
		&lightsClientBuffer[0], 0, spotTexLightsOffset + spotTexLightsSize);
	tilesBuffer.write(&tilesClientBuffer[0]);
	pointLightIndicesBuffer.write(plightIdsClientBuffer);
	spotLightIndicesBuffer.write(slightIdsClientBuffer);

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
	pointLightIndicesBuffer.setBinding(TILES_POINT_LIGHT_INDICES_BLOCK_BINDING);
	spotLightIndicesBuffer.setBinding(TILES_SPOT_LIGHT_INDICES_BLOCK_BINDING);

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
	ANKI_ASSERT(uboAlignment != MAX_PTR_SIZE);
	PtrSize size;

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
PtrSize Is::calcPointLightIndicesBufferSize() const
{
	return maxPointLightsPerTile * sizeof(U32) 
		* r->getTilesCount().x() * r->getTilesCount().y();
}

//==============================================================================
PtrSize Is::calcSpotLightIndicesBufferSize() const
{
	return (maxSpotLightsPerTile + maxSpotTexLightsPerTile) * sizeof(U32) 
		* r->getTilesCount().x() * r->getTilesCount().y();
}

//==============================================================================
void Is::blockSetupAndSanityCheck(const char* name, U binding, PtrSize size)
{
	const ShaderProgramUniformBlock* ublock;

	ublock = &lightPassProg->findUniformBlock(name);
	ublock->setBinding(binding);
	if(ublock->getSize() != size || ublock->getBinding() != binding)
	{
		throw ANKI_EXCEPTION("Problem with block");
	}
}

} // end namespace anki
