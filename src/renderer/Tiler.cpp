// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Tiler.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/resource/ShaderResource.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SceneNode.h>

namespace anki
{

//==============================================================================
Tiler::Tiler(Renderer* r)
	: RenderingPass(r)
{
}

//==============================================================================
Tiler::~Tiler()
{
	m_currentMinMax.destroy(getAllocator());
}

//==============================================================================
Error Tiler::init()
{
	Error err = initInternal();

	if(err)
	{
		ANKI_LOGE("Failed to init tiler");
	}

	return err;
}

//==============================================================================
Error Tiler::initInternal()
{
	// Load the program
	StringAuto pps(getAllocator());

	pps.sprintf("#define TILE_SIZE_X %u\n"
				"#define TILE_SIZE_Y %u\n"
				"#define TILES_COUNT_X %u\n"
				"#define TILES_COUNT_Y %u\n",
		TILE_SIZE,
		TILE_SIZE,
		m_r->getTileCountXY().x(),
		m_r->getTileCountXY().y());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_shader, "shaders/TilerMinMax.comp.glsl", pps.toCString(), "r_"));

	PipelineInitInfo pplineInit;
	pplineInit.m_shaders[U(ShaderType::COMPUTE)] = m_shader->getGrShader();
	m_ppline = getGrManager().newInstance<Pipeline>(pplineInit);

	// Allocate the buffers
	U pboSize = m_r->getTileCount() * sizeof(Vec2); // The pixel size

	for(U i = 0; i < m_outBuffers.getSize(); ++i)
	{
		// Create the buffer
		m_outBuffers[i] = getGrManager().newInstance<Buffer>(
			pboSize, BufferUsageBit::STORAGE, BufferAccessBit::CLIENT_MAP_READ);

		// Create graphics resources
		ResourceGroupInitInfo rcinit;
		rcinit.m_storageBuffers[0].m_buffer = m_outBuffers[i];
		rcinit.m_textures[0].m_texture = m_r->getMs().getDepthRt();

		m_rcGroups[i] = getGrManager().newInstance<ResourceGroup>(rcinit);
	}

	m_currentMinMax.create(getAllocator(), m_r->getTileCount());

	return ErrorCode::NONE;
}

//==============================================================================
void Tiler::run(CommandBufferPtr& cmd)
{
	// Issue the min/max job
	U pboIdx = m_r->getFrameCount() % m_outBuffers.getSize();

	cmd->bindPipeline(m_ppline);
	cmd->bindResourceGroup(m_rcGroups[pboIdx], 0, nullptr);

	cmd->dispatchCompute(
		m_r->getTileCountXY().x(), m_r->getTileCountXY().y(), 1);
}

//==============================================================================
void Tiler::prepareForVisibilityTests(const SceneNode& node)
{
	// Get the min max
	U size = m_r->getTileCount() * sizeof(Vec2);

	U buffIdx = max<U>(m_r->getFrameCount() % m_outBuffers.getSize(), 2u) - 2;
	BufferPtr& buff = m_outBuffers[buffIdx];
	void* mappedMem = buff->map(0, size, BufferAccessBit::CLIENT_MAP_READ);

	ANKI_ASSERT(mappedMem);
	memcpy(&m_currentMinMax[0], mappedMem, size);

	buff->unmap();

	// Convert the min max to view space
	const FrustumComponent& frc = node.getComponent<FrustumComponent>();
	const Vec4& projParams = frc.getProjectionParameters();

	for(Vec2& v : m_currentMinMax)
	{
		// Unproject
		v = projParams.z() / (projParams.w() + v);
	};

	// Other
	const MoveComponent& movec = node.getComponent<MoveComponent>();
	m_nearPlaneWspace =
		Plane(Vec4(0.0, 0.0, -1.0, 0.0), frc.getFrustum().getNear());
	m_nearPlaneWspace.transform(movec.getWorldTransform());

	m_viewProjMat = frc.getViewProjectionMatrix();
	m_near = frc.getFrustum().getNear();
}

//==============================================================================
Bool Tiler::test(const CollisionShape& cs, const Aabb& aabb) const
{
	// Compute the distance from the near plane
	F32 dist = aabb.testPlane(m_nearPlaneWspace);
	if(dist <= 0.0)
	{
		// Collides with the near plane. The following tests will fail
		return true;
	}

	dist += m_near;
	dist = -dist; // Because m_nearPlaneWspace has negatives

	// Find the tiles that affect it
	const Vec4& minv = aabb.getMin();
	const Vec4& maxv = aabb.getMax();
	Array<Vec4, 8> points;
	points[0] = minv.xyz1();
	points[1] = Vec4(minv.x(), maxv.y(), minv.z(), 1.0);
	points[2] = Vec4(minv.x(), maxv.y(), maxv.z(), 1.0);
	points[3] = Vec4(minv.x(), minv.y(), maxv.z(), 1.0);
	points[4] = maxv.xyz1();
	points[5] = Vec4(maxv.x(), minv.y(), maxv.z(), 1.0);
	points[6] = Vec4(maxv.x(), minv.y(), minv.z(), 1.0);
	points[7] = Vec4(maxv.x(), maxv.y(), minv.z(), 1.0);
	Vec2 min2(MAX_F32), max2(MIN_F32);
	for(Vec4& p : points)
	{
		p = m_viewProjMat * p;
		ANKI_ASSERT(p.w() > 0.0);
		p = p.perspectiveDivide();

		for(U i = 0; i < 2; ++i)
		{
			min2[i] = min(min2[i], p[i]);
			max2[i] = max(max2[i], p[i]);
		}
	}

	min2 = min2 * 0.5 + 0.5;
	max2 = max2 * 0.5 + 0.5;

	F32 tcountX = m_r->getTileCountXY().x();
	F32 tcountY = m_r->getTileCountXY().y();

	I xBegin = floor(tcountX * min2.x());
	xBegin = clamp<I>(xBegin, 0, tcountX);

	I xEnd = ceil(tcountX * max2.x());
	xEnd = min<U>(xEnd, tcountX);

	I yBegin = floor(tcountY * min2.y());
	yBegin = clamp<I>(yBegin, 0, tcountY);

	I yEnd = ceil(tcountY * max2.y());
	yEnd = min<I>(yEnd, tcountY);

	ANKI_ASSERT(
		xBegin >= 0 && xBegin <= tcountX && xEnd >= 0 && xEnd <= tcountX);
	ANKI_ASSERT(
		yBegin >= 0 && yBegin <= tcountX && yEnd >= 0 && yBegin <= tcountY);

	// Check every tile
	U visibleCount = (yEnd - yBegin) * (xEnd - xBegin);
	for(I y = yBegin; y < yEnd; y++)
	{
		for(I x = xBegin; x < xEnd; x++)
		{
			U tileIdx = y * tcountX + x;
			F32 tileMaxDist = m_currentMinMax[tileIdx].y();

			if(dist < tileMaxDist)
			{
				--visibleCount;
			}
		}
	}

	return visibleCount > 0;
}

} // end namespace anki
