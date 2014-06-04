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

class Light
{
public:
	Vec4 m_posRadius;
	Vec4 m_diffuseColorShadowmapId;
	Vec4 m_specularColorTexId;
};

class PointLight: public Light
{};

class SpotLight: public Light
{
public:
	Vec4 m_lightDir;
	Vec4 m_outerCosInnerCos;
	Array<Vec4, 4> m_extendPoints;
};

class SpotTexLight: public SpotLight
{
public:
	Mat4 m_texProjectionMat; ///< Texture projection matrix
};

class CommonUniforms
{
public:
	Vec4 m_projectionParams;
	Vec4 m_sceneAmbientColor;
	Vec4 m_groundLightDir;
};

} // end namespace shader

//==============================================================================
/// Write the lights to a client buffer
class WriteLightsJob: public ThreadpoolTask
{
public:
	shader::PointLight* m_pointLights = nullptr;
	shader::SpotLight* m_spotLights = nullptr;
	shader::SpotTexLight* m_spotTexLights = nullptr;

	U8* m_tileBuffer = nullptr;

	VisibilityTestResults::Container::const_iterator m_lightsBegin;
	VisibilityTestResults::Container::const_iterator m_lightsEnd;

	std::atomic<U32>* m_pointLightsCount = nullptr;
	std::atomic<U32>* m_spotLightsCount = nullptr;
	std::atomic<U32>* m_spotTexLightsCount = nullptr;
		
	Array2d<std::atomic<U32>, 
		ANKI_RENDERER_MAX_TILES_Y, 
		ANKI_RENDERER_MAX_TILES_X>
		* m_tilePointLightsCount = nullptr,
		* m_tileSpotLightsCount = nullptr,
		* m_tileSpotTexLightsCount = nullptr;

	Tiler* m_tiler = nullptr;
	Is* m_is = nullptr;

	/// Bin lights on CPU path
	Bool m_binLights = true;

	void operator()(ThreadId threadId, U threadsCount)
	{
		U ligthsCount = m_lightsEnd - m_lightsBegin;

		// Count job bounds
		PtrSize start, end;
		choseStartEnd(threadId, threadsCount, ligthsCount, start, end);

		// Run all lights
		for(U64 i = start; i < end; i++)
		{
			SceneNode* snode = (*(m_lightsBegin + i)).m_node;
			Light* light = staticCastPtr<Light*>(snode);

			switch(light->getLightType())
			{
			case Light::LT_POINT:
				{
					PointLight& l = 
						*staticCastPtr<PointLight*>(light);
					I pos = doLight(l);
					if(m_binLights && pos != -1)
					{
						binLight(l, pos);
					}
				}
				break;
			case Light::LT_SPOT:
				{
					SpotLight& l = *staticCastPtr<SpotLight*>(light);
					I pos = doLight(l);
					if(m_binLights && pos != -1)
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
		I i = m_pointLightsCount->fetch_add(1);
		if(i >= (I)m_is->m_maxPointLights)
		{
			return -1;
		}

		shader::PointLight& slight = m_pointLights[i];

		const Camera* cam = m_is->m_cam;
		ANKI_ASSERT(cam);
	
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
			cam->getViewMatrix());

		slight.m_posRadius = Vec4(pos, -1.0 / light.getRadius());
		slight.m_diffuseColorShadowmapId = light.getDiffuseColor();
		slight.m_specularColorTexId = light.getSpecularColor();

		return i;
	}

	/// Copy CPU spot light to GPU buffer
	I doLight(const SpotLight& light)
	{
		const Camera* cam = m_is->m_cam;
		Bool isTexLight = light.getShadowEnabled();
		I i;
		shader::SpotLight* baseslight = nullptr;

		if(isTexLight)
		{
			// Spot tex light

			i = m_spotTexLightsCount->fetch_add(1);
			if(i >= (I)m_is->m_maxSpotTexLights)
			{
				return -1;
			}

			shader::SpotTexLight& slight = m_spotTexLights[i];
			baseslight = &slight;

			// Write matrix
			static const Mat4 biasMat4(
				0.5, 0.0, 0.0, 0.5, 
				0.0, 0.5, 0.0, 0.5, 
				0.0, 0.0, 0.5, 0.5, 
				0.0, 0.0, 0.0, 1.0);
			// bias * proj_l * view_l * world_c
			slight.m_texProjectionMat = biasMat4 * light.getProjectionMatrix() 
				* Mat4::combineTransformations(light.getViewMatrix(),
				Mat4(cam->getWorldTransform()));

			// Transpose because of driver bug
			slight.m_texProjectionMat.transpose();
		}
		else
		{
			// Spot light without texture

			i = m_spotLightsCount->fetch_add(1);
			if(i >= (I)m_is->m_maxSpotLights)
			{
				return -1;
			}

			shader::SpotLight& slight = m_spotLights[i];
			baseslight = &slight;
		}

		// Write common stuff
		ANKI_ASSERT(baseslight);

		// Pos & dist
		Vec3 pos = light.getWorldTransform().getOrigin().getTransformed(
				cam->getViewMatrix());
		baseslight->m_posRadius = Vec4(pos, -1.0 / light.getDistance());

		// Diff color and shadowmap ID now
		baseslight->m_diffuseColorShadowmapId = 
			Vec4(light.getDiffuseColor().xyz(), (F32)light.getShadowMapIndex());

		// Spec color
		baseslight->m_specularColorTexId = light.getSpecularColor();

		// Light dir
		Vec3 lightDir = -light.getWorldTransform().getRotation().getZAxis();
		lightDir = cam->getViewMatrix().getRotationPart() * lightDir;
		baseslight->m_lightDir = Vec4(lightDir, 0.0);
		
		// Angles
		baseslight->m_outerCosInnerCos = Vec4(
			light.getOuterAngleCos(),
			light.getInnerAngleCos(), 
			1.0, 
			1.0);

		// extend points
		const PerspectiveFrustum& frustum = light.getFrustum();

		for(U i = 0; i < 4; i++)
		{
			Vec3 extendPoint = light.getWorldTransform().getOrigin() 
				+ frustum.getLineSegments()[i].getDirection();

			extendPoint.transform(cam->getViewMatrix());
			baseslight->m_extendPoints[i] = Vec4(extendPoint, 1.0);
		}

		return i;
	}

	// Bin point light
	void binLight(PointLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		m_tiler->test(light.getSpatialCollisionShape(), true, &bitset);

		// Bin to the correct tiles
		PtrSize tilesCount = 
			m_is->m_r->getTilesCount().x() * m_is->m_r->getTilesCount().y();
		for(U t = 0; t < tilesCount; t++)
		{
			// If not in tile bye
			if(!bitset.test(t))
			{
				continue;
			}

			U x = t % m_is->m_r->getTilesCount().x();
			U y = t / m_is->m_r->getTilesCount().x();

			U tilePos = (*m_tilePointLightsCount)[y][x].fetch_add(1);

			if(tilePos < m_is->m_maxPointLightsPerTile)
			{
				writeIndexToTileBuffer(0, pos, tilePos, t);
			}
		}
	}

	// Bin spot light
	void binLight(SpotLight& light, U pos)
	{
		// Do the tests
		Tiler::Bitset bitset;
		m_tiler->test(light.getSpatialCollisionShape(), true, &bitset);

		// Bin to the correct tiles
		PtrSize tilesCount = 
			m_is->m_r->getTilesCount().x() * m_is->m_r->getTilesCount().y();
		for(U t = 0; t < tilesCount; t++)
		{
			// If not in tile bye
			if(!bitset.test(t))
			{
				continue;
			}

			U x = t % m_is->m_r->getTilesCount().x();
			U y = t / m_is->m_r->getTilesCount().x();

			if(light.getShadowEnabled())
			{
				U tilePos = (*m_tileSpotTexLightsCount)[y][x].fetch_add(1);

				if(tilePos < m_is->m_maxSpotTexLightsPerTile)
				{
					writeIndexToTileBuffer(2, pos, tilePos, t);
				}
			}
			else
			{
				U tilePos = (*m_tileSpotLightsCount)[y][x].fetch_add(1);

				if(tilePos < m_is->m_maxSpotLightsPerTile)
				{
					writeIndexToTileBuffer(1, pos, tilePos, t);
				}
			}
		}
	}

	/// The "Tile" structure varies so this custom function writes to it
	void writeIndexToTileBuffer(
		U lightType, U lightIndex, U indexInTileArray, U tileIndex)
	{
		PtrSize offset = 
			tileIndex * m_is->calcTileSize() 
			+ sizeof(UVec4); // Tile header

		// Move to the correct light section
		switch(lightType)
		{
		case 0:
			break;
		case 1:
			offset += m_is->m_maxPointLightsPerTile * sizeof(U32);
			break;
		case 2:
			offset += 
				(m_is->m_maxSpotLightsPerTile + m_is->m_maxPointLightsPerTile)
				* sizeof(U32);
			break;
		default:
			ANKI_ASSERT(0);
		}

		// Move to the array offset
		offset += sizeof(U32) * indexInTileArray;

		// Write the lightIndex
		*((U32*)(m_tileBuffer + offset)) = lightIndex;
	}
};

//==============================================================================
Is::Is(Renderer* r)
	: RenderingPass(r), m_sm(r)
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
	m_groundLightEnabled = initializer.get("is.groundLightEnabled");
	m_maxPointLights = initializer.get("is.maxPointLights");
	m_maxSpotLights = initializer.get("is.maxSpotLights");
	m_maxSpotTexLights = initializer.get("is.maxSpotTexLights");

	if(m_maxPointLights < 1 || m_maxSpotLights < 1 || m_maxSpotTexLights < 1)
	{
		throw ANKI_EXCEPTION("Incorrect number of max lights");
	}

	m_maxPointLightsPerTile = initializer.get("is.maxPointLightsPerTile");
	m_maxSpotLightsPerTile = initializer.get("is.maxSpotLightsPerTile");
	m_maxSpotTexLightsPerTile = initializer.get("is.maxSpotTexLightsPerTile");

	if(m_maxPointLightsPerTile < 1 
		|| m_maxSpotLightsPerTile < 1 
		|| m_maxSpotTexLightsPerTile < 1
		|| m_maxPointLightsPerTile % 4 != 0 
		|| m_maxSpotLightsPerTile % 4 != 0
		|| m_maxSpotTexLightsPerTile % 4 != 0)
	{
		throw ANKI_EXCEPTION("Incorrect number of max lights per tile");
	}

	m_tileSize = calcTileSize();

	//
	// Init the passes
	//
	m_sm.init(initializer);

	//
	// Load the programs
	//
	std::stringstream pps;

	pps << "\n#define TILES_X_COUNT " << m_r->getTilesCount().x()
		<< "\n#define TILES_Y_COUNT " << m_r->getTilesCount().y()
		<< "\n#define TILES_COUNT " 
		<< (m_r->getTilesCount().x() * m_r->getTilesCount().y())
		<< "\n#define RENDERER_WIDTH " << m_r->getWidth()
		<< "\n#define RENDERER_HEIGHT " << m_r->getHeight()
		<< "\n#define MAX_POINT_LIGHTS_PER_TILE " 
		<< (U32)m_maxPointLightsPerTile
		<< "\n#define MAX_SPOT_LIGHTS_PER_TILE " 
		<< (U32)m_maxSpotLightsPerTile
		<< "\n#define MAX_SPOT_TEX_LIGHTS_PER_TILE " 
		<< (U32)m_maxSpotTexLightsPerTile
		<< "\n#define MAX_POINT_LIGHTS " << (U32)m_maxPointLights
		<< "\n#define MAX_SPOT_LIGHTS " << (U32)m_maxSpotLights 
		<< "\n#define MAX_SPOT_TEX_LIGHTS " << (U32)m_maxSpotTexLights
		<< "\n#define GROUND_LIGHT " << (U32)m_groundLightEnabled
		<< "\n#define TILES_BLOCK_BINDING " << TILES_BLOCK_BINDING
		<< "\n";

	if(m_sm.getPoissonEnabled())
	{
		pps << "#define POISSON 1\n";
	}
	else
	{
		pps << "#define POISSON 0\n";
	}

	// point light
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl); // Job for initialization

	m_lightVert.load(ProgramResource::createSrcCodeToCache(
		"shaders/IsLp.vert.glsl", pps.str().c_str(), "r_").c_str());
	m_lightFrag.load(ProgramResource::createSrcCodeToCache(
		"shaders/IsLp.frag.glsl", pps.str().c_str(), "r_").c_str());

	m_lightPpline = GlProgramPipelineHandle(jobs, 
		{m_lightVert->getGlProgram(), m_lightFrag->getGlProgram()});

	//
	// Create framebuffer
	//

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGB8,
			GL_RGB, GL_UNSIGNED_BYTE, 1, m_rt);

	m_fb = GlFramebufferHandle(jobs, {{m_rt, GL_COLOR_ATTACHMENT0}});

	//
	// Init the quad
	//
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0},
		{1.0, 0.0}, {0.0, 0.0}};

	GlClientBufferHandle tempBuff(jobs, sizeof(quadVertCoords), 
		(void*)&quadVertCoords[0][0]);

	m_quadPositionsVertBuff = GlBufferHandle(jobs, GL_ARRAY_BUFFER,
		tempBuff, 0);

	//
	// Create UBOs
	//
	const GLbitfield bufferBits = GL_DYNAMIC_STORAGE_BIT;

	m_commonBuff = GlBufferHandle(jobs, GL_SHADER_STORAGE_BUFFER, 
		sizeof(shader::CommonUniforms), bufferBits);

	m_lightsBuff = GlBufferHandle(jobs, GL_SHADER_STORAGE_BUFFER, 
		calcLightsBufferSize(), bufferBits);

	m_tilesBuff = GlBufferHandle(jobs, GL_SHADER_STORAGE_BUFFER,
		m_r->getTilesCount().x() * m_r->getTilesCount().y() * m_tileSize,
		bufferBits);

	// Last thing to do
	jobs.flush();
}

//==============================================================================
void Is::lightPass(GlJobChainHandle& jobs)
{
	Threadpool& threadPool = ThreadpoolSingleton::get();
	m_cam = &m_r->getSceneGraph().getActiveCamera();
	VisibilityTestResults& vi = m_cam->getVisibilityTestResults();

	//
	// Quickly get the lights
	//
	U visiblePointLightsCount = 0;
	U visibleSpotLightsCount = 0;
	U visibleSpotTexLightsCount = 0;
	Array<Light*, Sm::MAX_SHADOW_CASTERS> shadowCasters;

	for(auto& it : vi.m_lights)
	{
		Light* light = staticCastPtr<Light*>(it.m_node);
		switch(light->getLightType())
		{
		case Light::LT_POINT:
			++visiblePointLightsCount;
			break;
		case Light::LT_SPOT:
			{
				SpotLight* slight = staticCastPtr<SpotLight*>(light);
				
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
	clamp(visiblePointLightsCount, m_maxPointLights);
	clamp(visibleSpotLightsCount, m_maxSpotLights);
	clamp(visibleSpotTexLightsCount, m_maxSpotTexLights);

	U totalLightsCount = visiblePointLightsCount + visibleSpotLightsCount 
		+ visibleSpotTexLightsCount;

	ANKI_COUNTER_INC(RENDERER_LIGHTS_COUNT, U64(totalLightsCount));

	//
	// Do shadows pass
	//
	m_sm.run(&shadowCasters[0], visibleSpotTexLightsCount, jobs);

	//
	// Write the lights and tiles UBOs
	//
	GlManager& gl = GlManagerSingleton::get();
	U32 blockAlignment = gl.getBufferOffsetAlignment(m_lightsBuff.getTarget());

	// Get the offsets and sizes of each uniform block
	PtrSize pointLightsOffset = 0;
	PtrSize pointLightsSize = getAlignedRoundUp(
		blockAlignment, sizeof(shader::PointLight) * visiblePointLightsCount);

	PtrSize spotLightsOffset = pointLightsSize;
	PtrSize spotLightsSize = getAlignedRoundUp(
		blockAlignment, sizeof(shader::SpotLight) * visibleSpotLightsCount);

	PtrSize spotTexLightsOffset = spotLightsOffset + spotLightsSize;
	PtrSize spotTexLightsSize = getAlignedRoundUp(blockAlignment, 
		sizeof(shader::SpotTexLight) * visibleSpotTexLightsCount);

	ANKI_ASSERT(
		spotTexLightsOffset + spotTexLightsSize <= calcLightsBufferSize());

	// Fire the super jobs
	Array<WriteLightsJob, Threadpool::MAX_THREADS> tjobs;

	GlClientBufferHandle lightsClientBuff;
	if(totalLightsCount > 0)
	{
		lightsClientBuff = GlClientBufferHandle(
			jobs, spotTexLightsOffset + spotTexLightsSize, nullptr);
	}

	GlClientBufferHandle tilesClientBuff(jobs, m_tilesBuff.getSize(), nullptr);

	std::atomic<U32> pointLightsAtomicCount(0);
	std::atomic<U32> spotLightsAtomicCount(0);
	std::atomic<U32> spotTexLightsAtomicCount(0);

	Array2d<std::atomic<U32>, 
		ANKI_RENDERER_MAX_TILES_Y, 
		ANKI_RENDERER_MAX_TILES_X> 
		tilePointLightsCount,
		tileSpotLightsCount,
		tileSpotTexLightsCount;

	for(U y = 0; y < m_r->getTilesCount().y(); y++)
	{
		for(U x = 0; x < m_r->getTilesCount().x(); x++)
		{
			tilePointLightsCount[y][x].store(0);
			tileSpotLightsCount[y][x].store(0);
			tileSpotTexLightsCount[y][x].store(0);
		}
	}

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		WriteLightsJob& job = tjobs[i];

		if(i == 0)
		{
			if(totalLightsCount > 0)
			{
				job.m_pointLights = (shader::PointLight*)(
					(U8*)lightsClientBuff.getBaseAddress() 
					+ pointLightsOffset);
				job.m_spotLights = (shader::SpotLight*)(
					(U8*)lightsClientBuff.getBaseAddress() 
					+ spotLightsOffset);
				job.m_spotTexLights = (shader::SpotTexLight*)(
					(U8*)lightsClientBuff.getBaseAddress() 
					+ spotTexLightsOffset);
			}

			job.m_tileBuffer = (U8*)tilesClientBuff.getBaseAddress();

			job.m_lightsBegin = vi.m_lights.begin();
			job.m_lightsEnd = vi.m_lights.end();

			job.m_pointLightsCount = &pointLightsAtomicCount;
			job.m_spotLightsCount = &spotLightsAtomicCount;
			job.m_spotTexLightsCount = &spotTexLightsAtomicCount;

			job.m_tilePointLightsCount = &tilePointLightsCount;
			job.m_tileSpotLightsCount = &tileSpotLightsCount;
			job.m_tileSpotTexLightsCount = &tileSpotTexLightsCount;

			job.m_tiler = &m_r->getTiler();
			job.m_is = this;
		}
		else
		{
			// Just copy from the first job. All jobs have the same data

			job = tjobs[0];
		}

		threadPool.assignNewTask(i, &job);
	}

	// In the meantime set the state
	setState(jobs);

	// Sync
	threadPool.waitForAllThreadsToFinish();

	// Write the light count for each tile
	for(U y = 0; y < m_r->getTilesCount().y(); y++)
	{
		for(U x = 0; x < m_r->getTilesCount().x(); x++)
		{
			UVec4* vec;

			vec = (UVec4*)(
				(U8*)tilesClientBuff.getBaseAddress() 
				+ (y * m_r->getTilesCount().x() + x) * m_tileSize);

			vec->x() = tilePointLightsCount[y][x].load();
			clamp(vec->x(), m_maxPointLightsPerTile);

			vec->y() = 0;

			vec->z() = tileSpotLightsCount[y][x].load();
			clamp(vec->z(), m_maxSpotLightsPerTile);

			vec->w() = tileSpotTexLightsCount[y][x].load();
			clamp(vec->w(), m_maxSpotTexLightsPerTile);
		}
	}

	// Write BOs
	if(totalLightsCount > 0)
	{
		m_lightsBuff.write(jobs,
			lightsClientBuff, 0, 0, spotTexLightsOffset + spotTexLightsSize);
	}
	m_tilesBuff.write(jobs, tilesClientBuff, 0, 0, tilesClientBuff.getSize());

	//
	// Setup uniforms
	//

	// shader prog

	updateCommonBlock(jobs);

	m_commonBuff.bindShaderBuffer(jobs, COMMON_UNIFORMS_BLOCK_BINDING);

	if(pointLightsSize > 0)
	{
		m_lightsBuff.bindShaderBuffer(jobs, 
			pointLightsOffset, pointLightsSize, POINT_LIGHTS_BLOCK_BINDING);
	}

	if(spotLightsSize > 0)
	{
		m_lightsBuff.bindShaderBuffer(jobs, 
			spotLightsOffset, spotLightsSize, SPOT_LIGHTS_BLOCK_BINDING);
	}

	if(spotTexLightsSize > 0)
	{
		m_lightsBuff.bindShaderBuffer(jobs,
			spotTexLightsOffset, spotTexLightsSize, 
			SPOT_TEX_LIGHTS_BLOCK_BINDING);
	}

	m_tilesBuff.bindShaderBuffer(jobs, TILES_BLOCK_BINDING); 

	// The binding points should much the shader
	jobs.bindTextures(0, {
		m_r->getMs()._getRt0(), 
		m_r->getMs()._getRt1(), 
		m_r->getMs()._getDepthRt(),
		m_sm.m_sm2DArrayTex});

	//
	// Draw
	//

	m_lightPpline.bind(jobs);

	m_quadPositionsVertBuff.bindVertexBuffer(jobs, 
		2, GL_FLOAT, false, 0, 0, 0);

	GlDrawcallArrays dc(GL_TRIANGLE_STRIP, 4, 
		m_r->getTilesCount().x() * m_r->getTilesCount().y());

	dc.draw(jobs);
}

//==============================================================================
void Is::setState(GlJobChainHandle& jobs)
{
	Bool drawToDefaultFbo = !m_r->getPps().getEnabled() 
		&& !m_r->getDbg().getEnabled() 
		&& !m_r->getIsOffscreen()
		&& m_r->getRenderingQuality() == 1.0;

	if(drawToDefaultFbo)
	{
		m_r->getDefaultFramebuffer().bind(jobs, true);
		jobs.setViewport(0, 0, m_r->getWindowWidth(), m_r->getWindowHeight());
	}
	else
	{
		m_fb.bind(jobs, true);

		jobs.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}
}

//==============================================================================
void Is::run(GlJobChainHandle& jobs)
{
	// Do the light pass including the shadow passes
	lightPass(jobs);
}

//==============================================================================
void Is::updateCommonBlock(GlJobChainHandle& jobs)
{
	SceneGraph& scene = m_r->getSceneGraph();

	GlClientBufferHandle cbuff(jobs, sizeof(shader::CommonUniforms), nullptr);
	shader::CommonUniforms& blk = 
		*(shader::CommonUniforms*)cbuff.getBaseAddress();

	// Start writing
	blk.m_projectionParams = m_r->getProjectionParameters();

	blk.m_sceneAmbientColor = scene.getAmbientColor();

	Vec3 groundLightDir;
	if(m_groundLightEnabled)
	{
		blk.m_groundLightDir = 
			Vec4(-m_cam->getViewMatrix().getColumn(1).xyz(), 1.0);
	}

	m_commonBuff.write(jobs, cbuff, 0, 0, cbuff.getSize());
}

//==============================================================================
PtrSize Is::calcLightsBufferSize() const
{
	U32 buffAlignment = GlManagerSingleton::get().getBufferOffsetAlignment(
		GL_SHADER_STORAGE_BUFFER);
	PtrSize size;

	size = getAlignedRoundUp(
		buffAlignment,
		m_maxPointLights * sizeof(shader::PointLight));

	size += getAlignedRoundUp(
		buffAlignment,
		m_maxSpotLights * sizeof(shader::SpotLight));

	size += getAlignedRoundUp(
		buffAlignment,
		m_maxSpotTexLights * sizeof(shader::SpotTexLight));

	return size;
}

//==============================================================================
PtrSize Is::calcTileSize() const
{
	return 
		sizeof(U32) * 4 
		+ m_maxPointLightsPerTile * sizeof(U32)
		+ m_maxSpotLightsPerTile * sizeof(U32)
		+ m_maxSpotTexLightsPerTile * sizeof(U32);
}

} // end namespace anki
