// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/core/Config.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/scene/ReflectionProxyComponent.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

struct ShaderReflectionProxy
{
	Vec4 m_plane;
	Vec4 m_negPlane;
	Array<Vec4, 4> m_quadPoints;
	Array<Vec4, 4> m_edgeCrossProd;
};

struct ShaderReflectionProbe
{
	Vec3 m_pos;
	F32 m_radiusSq;
	F32 m_cubemapIndex;
	U32 _m_pading[3];
};

//==============================================================================
// Ir                                                                          =
//==============================================================================

//==============================================================================
Ir::~Ir()
{
	m_cacheEntries.destroy(getAllocator());
}

//==============================================================================
Error Ir::init(const ConfigSet& initializer)
{
	ANKI_LOGI("Initializing IR (Image Reflections)");
	m_fbSize = initializer.getNumber("ir.rendererSize");

	if(m_fbSize < Renderer::TILE_SIZE)
	{
		ANKI_LOGE("Too low ir.rendererSize");
		return ErrorCode::USER_DATA;
	}

	m_cubemapArrSize = initializer.getNumber("ir.cubemapTextureArraySize");

	if(m_cubemapArrSize < 2)
	{
		ANKI_LOGE("Too low ir.cubemapTextureArraySize");
		return ErrorCode::USER_DATA;
	}

	m_cacheEntries.create(getAllocator(), m_cubemapArrSize);

	// Init the renderer
	Config config;
	config.set("dbg.enabled", false);
	config.set("is.sm.bilinearEnabled", true);
	config.set("is.groundLightEnabled", false);
	config.set("is.sm.enabled", false);
	config.set("is.sm.maxLights", 8);
	config.set("is.sm.poissonEnabled", false);
	config.set("is.sm.resolution", 16);
	config.set("lf.maxFlares", 8);
	config.set("pps.enabled", false);
	config.set("renderingQuality", 1.0);
	config.set("width", m_fbSize);
	config.set("height", m_fbSize);
	config.set("lodDistance", 10.0);
	config.set("samples", 1);
	config.set("ir.enabled", false); // Very important to disable that

	ANKI_CHECK(m_nestedR.init(&m_r->getThreadPool(),
		&m_r->getResourceManager(), &m_r->getGrManager(), m_r->getAllocator(),
		m_r->getFrameAllocator(), config, m_r->getGlobalTimestampPtr()));

	// Init the texture
	TextureInitializer texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_depth = m_cubemapArrSize;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_format = Is::RT_PIXEL_FORMAT;
	texinit.m_mipmapsCount = 1;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;

	m_cubemapArr = getGrManager().newInstance<Texture>(texinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::run(CommandBufferPtr cmdb)
{
	FrustumComponent& frc = m_r->getActiveFrustumComponent();
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	// Do the proxies
	void* data = getGrManager().allocateFrameHostVisibleMemory(
		sizeof(ShaderReflectionProxy) * visRez.getReflectionProxyCount()
		+ sizeof(UVec4), BufferUsage::STORAGE, m_proxiesToken);

	UVec4* counts = reinterpret_cast<UVec4*>(data);
	counts->x() = visRez.getReflectionProxyCount();
	counts->y() = visRez.getReflectionProbeCount();

	const VisibleNode* it = visRez.getReflectionProxiesBegin();
	const VisibleNode* end = visRez.getReflectionProxiesEnd();
	ShaderReflectionProxy* proxies = reinterpret_cast<ShaderReflectionProxy*>(
		counts + 1);

	while(it != end)
	{
		const ReflectionProxyComponent& proxyc =
			it->m_node->getComponent<ReflectionProxyComponent>();
		Plane plane = proxyc.getPlane();
		plane.transform(Transform(frc.getViewMatrix()));

		proxies->m_plane = Vec4(plane.getNormal().xyz(), plane.getOffset());
		proxies->m_negPlane = Vec4(-plane.getNormal().xyz(), plane.getOffset());

		for(U i = 0; i < 4; ++i)
		{
			proxies->m_quadPoints[i] =
				frc.getViewMatrix() * proxyc.getVertices()[i].xyz1();
		}

		for(U i = 0; i < 4; ++i)
		{
			U next = (i < 3) ? (i + 1) : 0;
			proxies->m_edgeCrossProd[i] = plane.getNormal().cross(
				proxies->m_quadPoints[next] - proxies->m_quadPoints[i]);
		}

		++it;
		++proxies;
	}

	// Do the probes
	it = visRez.getReflectionProbesBegin();
	end = visRez.getReflectionProbesEnd();

	data = getGrManager().allocateFrameHostVisibleMemory(
		sizeof(ShaderReflectionProbe) * visRez.getReflectionProbeCount()
		+ sizeof(Mat3x4),
		BufferUsage::STORAGE, m_probesToken);

	Mat3x4* invViewRotation = static_cast<Mat3x4*>(data);
	*invViewRotation =
		Mat3x4(frc.getViewMatrix().getInverse().getRotationPart());

	ShaderReflectionProbe* probes = reinterpret_cast<ShaderReflectionProbe*>(
		invViewRotation + 1);

	while(it != end)
	{
		ANKI_CHECK(renderReflection(*it->m_node, *probes));
		++it;
		++probes;
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::renderReflection(SceneNode& node, ShaderReflectionProbe& shaderProb)
{
	const FrustumComponent& frc = m_r->getActiveFrustumComponent();
	const ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	// Get cache entry
	Bool render = false;
	U entry;
	findCacheEntry(node, entry, render);

	// Write shader var
	shaderProb.m_pos = (frc.getViewMatrix() * reflc.getPosition().xyz1()).xyz();
	shaderProb.m_radiusSq = reflc.getRadius() * reflc.getRadius();
	shaderProb.m_cubemapIndex = entry;

	// Render cubemap
	if(render)
	{
		for(U i = 0; i < 6; ++i)
		{
			Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT> cmdb;
			for(U j = 0; j < cmdb.getSize(); ++j)
			{
				cmdb[j] = getGrManager().newInstance<CommandBuffer>();
			}

			// Render
			ANKI_CHECK(m_nestedR.render(node, i, cmdb));

			// Copy textures
			cmdb[cmdb.getSize() - 1]->copyTextureToTexture(
				m_nestedR.getIs().getRt(), 0, 0, m_cubemapArr, 6 * entry + i,
				0);

			// Flush
			for(U j = 0; j < cmdb.getSize(); ++j)
			{
				cmdb[j]->flush();
			}
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Ir::findCacheEntry(SceneNode& node, U& entry, Bool& render)
{
	CacheEntry* it = m_cacheEntries.getBegin();
	const CacheEntry* const end = m_cacheEntries.getEnd();

	CacheEntry* canditate = nullptr;
	CacheEntry* empty = nullptr;
	CacheEntry* kick = nullptr;
	Timestamp kickTime = MAX_TIMESTAMP;

	while(it != end)
	{
		if(it->m_node == &node)
		{
			// Already there
			canditate = it;
			break;
		}
		else if(empty == nullptr && it->m_node == nullptr)
		{
			// Found empty
			empty = it;
		}
		else if(it->m_timestamp < kickTime)
		{
			// Found one to kick
			kick = it;
			kickTime = it->m_timestamp;
		}

		++it;
	}

	if(canditate)
	{
		// Update timestamp
		canditate->m_timestamp = getGlobalTimestamp();
		it = canditate;
		render = false;
	}
	else if(empty)
	{
		ANKI_ASSERT(empty->m_node == nullptr);
		empty->m_node = &node;
		empty->m_timestamp = getGlobalTimestamp();

		it = empty;
		render = true;
	}
	else if(kick)
	{
		kick->m_node = &node;
		kick->m_timestamp = getGlobalTimestamp();

		it = kick;
		render = true;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	entry = it - m_cacheEntries.getBegin();
}

} // end namespace anki

