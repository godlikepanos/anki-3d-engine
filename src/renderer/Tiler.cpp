// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderResource.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Rtti.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
class UpdatePlanesPerspectiveCameraTask: public Threadpool::Task
{
public:
	Tiler* m_tiler = nullptr;
	Camera* m_cam = nullptr;
	Bool m_frustumChanged;

	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		m_tiler->update(threadId, threadsCount, *m_cam, m_frustumChanged);

		return ErrorCode::NONE;
	}
};

//==============================================================================
static Vec4 unproject(const F32 depth, const Vec2& ndc, const Vec4& projParams)
{
	Vec4 view;
	F32 z = projParams.z() / (projParams.w() + depth);
	Vec2 viewxy = ndc * projParams.xy() * z;
	view.x() = viewxy.x();
	view.y() = viewxy.y();
	view.z() = z;
	view.w() = 0.0;

	return view;
}

//==============================================================================
// Tiler                                                                       =
//==============================================================================

//==============================================================================
Tiler::Tiler(Renderer* r)
:	RenderingPass(r)
{}

//==============================================================================
Tiler::~Tiler()
{
	m_allPlanes.destroy(getAllocator());
	m_prevMinMaxDepth.destroy(getAllocator());
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
	m_enableGpuTests = false;

	// Load the program
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define TILES_X_COUNT %u\n"
		"#define TILES_Y_COUNT %u\n"
		"#define RENDERER_WIDTH %u\n"
		"#define RENDERER_HEIGHT %u\n",
		m_r->getTilesCount().x(),
		m_r->getTilesCount().y(),
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(
		m_frag.loadToCache(&getResourceManager(), 
		"shaders/TilerMinMax.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(
		m_r->createDrawQuadPipeline(m_frag->getGrShader(), m_ppline));

	// Create FB
	ANKI_CHECK(
		m_r->createRenderTarget(m_r->getTilesCount().x(), 
		m_r->getTilesCount().y(), 
		PixelFormat(ComponentFormat::R32G32, TransformFormat::UINT), 
		1, SamplingFilter::NEAREST, 1, m_rt));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	ANKI_CHECK(m_fb.create(&getGrManager(), fbInit));

	// Init planes. One plane for each direction, plus near/far plus the world
	// space of those
	U planesCount = 
		(m_r->getTilesCount().x() - 1) * 2 // planes J
		+ (m_r->getTilesCount().y() - 1) * 2  // planes I
		+ (m_r->getTilesCount().x() * m_r->getTilesCount().y() * 2); // near+far

	m_allPlanes.create(getAllocator(), planesCount);

	Plane* base = &m_allPlanes[0];
	U count = 0;
	using S = SArray<Plane>;
	
	m_planesX = std::move(S(base + count, m_r->getTilesCount().x() - 1));
	count += m_planesX.getSize();

	m_planesY = std::move(S(base + count, m_r->getTilesCount().y() - 1));
	count += m_planesY.getSize();

	m_planesXW = std::move(S(base + count, m_r->getTilesCount().x() - 1));
	count += m_planesXW.getSize();

	m_planesYW = std::move(S(base + count, m_r->getTilesCount().y() - 1));
	count += m_planesYW.getSize();

	m_nearPlanesW = std::move(S(base + count, 
		m_r->getTilesCount().x() * m_r->getTilesCount().y()));
	count += m_nearPlanesW.getSize();

	m_farPlanesW = std::move(S(base + count, 
		m_r->getTilesCount().x() * m_r->getTilesCount().y()));

	ANKI_ASSERT(count + m_farPlanesW.getSize() == m_allPlanes.getSize());

	ANKI_CHECK(initPbos());

	m_prevMinMaxDepth.create(getAllocator(),
		m_r->getTilesCount().x() * m_r->getTilesCount().y(), Vec2(0.0, 1.0));

	return ErrorCode::NONE;
}

//==============================================================================
Error Tiler::initPbos()
{
	// Allocate the buffers
	U pboSize = m_r->getTilesCount().x() * m_r->getTilesCount().y();
	pboSize *= sizeof(Vec2); // The pixel size

	for(U i = 0; i < m_pbos.getSize(); ++i)
	{
		ANKI_CHECK(
			m_pbos[i].create(&getGrManager(), GL_PIXEL_PACK_BUFFER, nullptr, 
			pboSize,
			GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
	}

	// Get persistent address
	for(U i = 0; i < m_pbos.getSize(); ++i)
	{
		m_pbosAddress[i] = 
			static_cast<Vec2*>(m_pbos[i].getPersistentMappingAddress());
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Tiler::runMinMax(TextureHandle& depthMap,
	CommandBufferHandle& cmd)
{
	if(m_enableGpuTests)
	{
		// Issue the min/max job
		m_fb.bind(cmd);
		m_ppline.bind(cmd);
		depthMap.bind(cmd, 0);
		cmd.setViewport(
			0, 0, m_r->getTilesCount().x(), m_r->getTilesCount().y());

		m_r->drawQuad(cmd);

		// Issue the async pixel read
		U pboIdx = getGlobalTimestamp() % m_pbos.getSize();
		cmd.copyTextureToBuffer(m_rt, m_pbos[pboIdx]);
	}
}

//==============================================================================
void Tiler::updateTiles(Camera& cam)
{
	//
	// Issue parallel jobs
	//
	Array<UpdatePlanesPerspectiveCameraTask, Threadpool::MAX_THREADS> jobs;
	const FrustumComponent& camFr = cam.getComponent<FrustumComponent>();
	U32 camTimestamp = camFr.getTimestamp();

	// Do a job that transforms only the planes when:
	// - it is the same camera as before and
	// - the camera frustum have not changed
	Bool frustumChanged =
		camTimestamp >= m_planes4UpdateTimestamp || m_prevCam != &cam;

	Threadpool& threadPool = m_r->_getThreadpool();
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].m_tiler = this;
		jobs[i].m_cam = &cam;
		jobs[i].m_frustumChanged = frustumChanged;
		threadPool.assignNewTask(i, &jobs[i]);
	}

	// Update timestamp
	if(frustumChanged)
	{
		m_planes4UpdateTimestamp = getGlobalTimestamp();
	}

	// Sync threads
	Error err = threadPool.waitForAllThreadsToFinish();
	(void)err;

	// 
	// Misc
	// 
	m_prevCam = &cam;
}

//==============================================================================
Bool Tiler::test(TestParameters& params) const
{
	ANKI_ASSERT(params.m_collisionShape);
	ANKI_ASSERT(params.m_collisionShapeBox);

	if(params.m_output)
	{
		params.m_output->m_count = 0;
	}

	// Call the recursive function or the fast path
	U count = 0;
#if 1
	if(isa<Sphere>(params.m_collisionShape))
	{
		testFastSphere(
			dcast<const Sphere&>(*params.m_collisionShape),
			*params.m_collisionShapeBox,
			params.m_output, 
			count);
	}
	else
#endif
	{
		testRange(
			*params.m_collisionShape, 
			params.m_nearPlane, 
			0, m_r->getTilesCount().y(), 0, m_r->getTilesCount().x(), 
			params.m_output,
			count);
	}

	if(params.m_output)
	{
		ANKI_ASSERT(count == params.m_output->m_count);
	}

	return count > 0;
}

//==============================================================================
void Tiler::testRange(const CollisionShape& cs, Bool nearPlane,
	U yFrom, U yTo, U xFrom, U xTo, TestResult* visible, U& count) const
{
	U my = (yTo - yFrom) / 2;
	U mx = (xTo - xFrom) / 2;

	// Handle final
	if(ANKI_UNLIKELY(my == 0 && mx == 0))
	{
		Bool inside = true;

		if(m_enableGpuTests && getGlobalTimestamp() >= m_pbos.getSize())
		{
			U tileId = m_r->getTilesCount().x() * yFrom + xFrom;

			if(cs.testPlane(m_farPlanesW[tileId]) >= 0.0)
			{
				// Inside on far but check near

				if(nearPlane)
				{
					if(cs.testPlane(m_nearPlanesW[tileId]) >= 0)
					{
						// Inside
					}
					else
					{
						inside = false;
					}
				}
				else
				{
					// Inside
				}
			}
			else
			{
				inside = false;
			}
		}

		if(inside)
		{
			if(visible)
			{
				visible->pushBack(xFrom, yFrom);
			}

			++count;
		}

		return;
	}

	// Handle the edge case
	if(ANKI_UNLIKELY(mx == 0 || my == 0))
	{
		if(mx == 0)
		{
			const Plane& topPlane = m_planesYW[yFrom + my - 1];
			F32 test = cs.testPlane(topPlane);

			if(test <= 0.0)
			{
				testRange(cs, nearPlane, 
					yFrom, yFrom + my, xFrom, xTo, visible, count);
			}

			if(test >= 0.0)
			{
				testRange(cs, nearPlane, 
					yFrom + my, yTo, xFrom, xTo, visible, count);
			}
		}
		else
		{
			const Plane& rightPlane = m_planesXW[xFrom + mx - 1];
			F32 test = cs.testPlane(rightPlane);

			if(test <= 0.0)
			{
				testRange(cs, nearPlane, 
					yFrom, yTo, xFrom, xFrom + mx, visible, count);
			}

			if(test >= 0.0)
			{
				testRange(cs, nearPlane, 
					yFrom, yTo, xFrom + mx, xTo, visible, count);
			}
		}

		return;
	}

	// Do the checks
	Bool inside[2][2] = {{false, false}, {false, false}};

	// Top looking plane check
	{
		// Pick the correct top lookin plane (y)
		const Plane& topPlane = m_planesYW[yFrom + my - 1];

		F32 test = cs.testPlane(topPlane);
		if(test < 0.0)
		{
			inside[0][0] = inside[0][1] = true;
		}
		else if(test > 0.0)
		{
			inside[1][0] = inside[1][1] = true;
		}
		else
		{
			// Possibly all inside
			for(U i = 0; i < 2; i++)
			{
				for(U j = 0; j < 2; j++)
				{
					inside[i][j] = true;
				}
			}
		}
	}

	// Right looking plane check
	{
		// Pick the correct right looking plane (x)
		const Plane& rightPlane = m_planesXW[xFrom + mx - 1];

		F32 test = cs.testPlane(rightPlane);
		if(test < 0.0)
		{
			inside[0][1] = inside[1][1] = false;
		}
		else if(test > 0.0)
		{
			inside[0][0] = inside[1][0] = false;
		}
		else
		{
			// Do nothing and keep the top looking plane check results
		}
	}

	// Now move lower to the hierarchy
	if(inside[0][0])
	{
		testRange(cs, nearPlane,
			yFrom, yFrom + my,
			xFrom, xFrom + mx,
			visible, count);
	}

	if(inside[0][1])
	{
		testRange(cs, nearPlane,
			yFrom, yFrom + my,
			xFrom + mx, xTo,
			visible, count);
	}

	if(inside[1][0])
	{
		testRange(cs, nearPlane,
			yFrom + my, yTo,
			xFrom, xFrom + mx,
			visible, count);
	}

	if(inside[1][1])
	{
		testRange(cs, nearPlane,
			yFrom + my, yTo,
			xFrom + mx, xTo,
			visible, count);
	}
}

//==============================================================================
void Tiler::testFastSphere(const Sphere& s, const Aabb& aabb,
	TestResult* visible, U& count) const
{
	const Camera& cam = m_r->getSceneGraph().getActiveCamera();
	const FrustumComponent& frc = cam.getComponent<FrustumComponent>();
	const Mat4& vp = frc.getViewProjectionMatrix();
	const Mat4& v = frc.getViewMatrix();
	const Transform& trf = 
		cam.getComponent<MoveComponent>().getWorldTransform();

	const Vec4& scent = s.getCenter();
	const F32 srad = s.getRadius();

	// Do a quick check
	Vec4 eye = trf.getOrigin() - scent;
	if(ANKI_UNLIKELY(eye.getLengthSquared() <= srad * srad))
	{
		// Camera totaly inside the sphere
		for(U y = 0; y < m_r->getTilesCount().y(); ++y)
		{
			for(U x = 0; x < m_r->getTilesCount().x(); ++x)
			{
				visible->pushBack(x, y);
				++count;
			}
		}
		return;
	}

	// Compute projection points
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
		p = vp * p;
		p /= abs(p.w());

		for(U i = 0; i < 2; ++i)
		{
			min2[i] = min(min2[i], p[i]);
			max2[i] = max(max2[i], p[i]);
		}
	}

	min2 = min2 * 0.5 + 0.5;
	max2 = max2 * 0.5 + 0.5;

	// Do a box test
	F32 tcountX = m_r->getTilesCount().x();
	F32 tcountY = m_r->getTilesCount().y();

	I xFrom = floor(tcountX * min2.x());
	xFrom = clamp<I>(xFrom, 0, m_r->getTilesCount().x());

	I xTo = ceil(tcountX * max2.x());
	xTo = min<U>(xTo, m_r->getTilesCount().x());

	I yFrom = floor(tcountY * min2.y());
	yFrom = clamp<I>(yFrom, 0, m_r->getTilesCount().y());

	I yTo = ceil(tcountY * max2.y());
	yTo = min<I>(yTo, m_r->getTilesCount().y());


	ANKI_ASSERT(xFrom >= 0 && xFrom <= tcountX 
		&& xTo >= 0 && xTo <= tcountX);
	ANKI_ASSERT(yFrom >= 0 && yFrom <= tcountX 
		&& yTo >= 0 && yFrom <= tcountY);

	Vec2 tileSize(1.0 / tcountX, 1.0 / tcountY);

	Vec4 a = vp * s.getCenter().xyz1();
	Vec2 c = a.xy() / a.w();
	c = c * 0.5 + 0.5;

	Vec4 sphereCenterVSpace = (v * scent.xyz1()).xyz0();

	for(I y = yFrom; y < yTo; ++y)
	{
		for(I x = xFrom; x < xTo; ++x)
		{
			// Do detailed tests

			Vec2 tileMin = Vec2(x, y) * tileSize;
			Vec2 tileMax = Vec2(x + 1, y + 1) * tileSize;
			
			// Find closest point of sphere center and tile
			Vec2 cp(0.0);
			for(U i = 0; i < 2; ++i)
			{
				if(c[i] > tileMax[i])
				{
					cp[i] = tileMax[i];
				}
				else if (c[i] < tileMin[i])
				{
					cp[i] = tileMin[i];
				}
				else
				{
					// the c lies between min and max
					cp[i] = c[i];
				}
			}

			// Unproject the closest point to view space
			Vec4 view = unproject(
				1.0, cp * 2.0 - 1.0, frc.getProjectionParameters());

			// Do a simple ray-sphere test
			Vec4 dir = view;
			Vec4 proj = sphereCenterVSpace.getProjection(dir);
			F32 lenSq = (sphereCenterVSpace - proj).getLengthSquared();
			Bool inside = lenSq <= (srad * srad);

			if(inside)
			{
				visible->pushBack(x, y);
				++count;
			}
		}
	}
}

//==============================================================================
void Tiler::update(U32 threadId, PtrSize threadsCount, 
	Camera& cam, Bool frustumChanged)
{
	PtrSize start, end;
	const MoveComponent& move = cam.getComponent<MoveComponent>();
	const FrustumComponent& frc = cam.getComponent<FrustumComponent>();
	ANKI_ASSERT(frc.getFrustum().getType() == Frustum::Type::PERSPECTIVE);

	const Transform& trf = move.getWorldTransform();
	const Vec4& projParams = frc.getProjectionParameters();

	if(frustumChanged)
	{
		// Re-calculate the planes in local space

		// First the top looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().y() - 1,
			start, end);

		for(U i = start; i < end; i++)
		{
			calcPlaneY(i, projParams);

			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().x() - 1,
			start, end);

		for(U j = start; j < end; j++)
		{
			calcPlaneX(j, projParams);

			m_planesXW[j] = m_planesX[j].getTransformed(trf);
		}
	}
	else
	{
		// Only transform planes

		// First the top looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().y() - 1,
			start, end);

		for(U i = start; i < end; i++)
		{
			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().x() - 1,
			start, end);

		for(U j = start; j < end; j++)
		{
			m_planesXW[j] = m_planesX[j].getTransformed(trf);
		}
	}

	// Update the near far planes
	if(m_enableGpuTests && getGlobalTimestamp() >= m_pbos.getSize())
	{
		const U tilesCount = 
			m_r->getTilesCount().x() * m_r->getTilesCount().y();

		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, tilesCount, start, end);

		// Setup pixel buffer
		U pboIdx = 
			(getGlobalTimestamp() - m_pbos.getSize() + 1) % m_pbos.getSize();
		SArray<Vec2> pixels(m_pbosAddress[pboIdx], tilesCount);

		for(U k = start; k < end; ++k)
		{
			Vec2 minMax = pixels[k];

			// Predict the min/max for the current frame.
			// v0 = unproject(minDepth)
			// p0 = PrevCamTrf * v0 where p0 is in world space
			// p1 = CrntCamTrf * p0
			// n1 = project(p1, CamProj)
			// Using n1 get the tile depth and use that to set the planes
			U x = k % m_r->getTilesCount().x();
			U y = k / m_r->getTilesCount().x();
			Vec2 tileCountf(m_r->getTilesCount().x(), m_r->getTilesCount().y());
			Vec2 ndc0 = Vec2(x, y) / tileCountf * 2.0 - 1.0;
			for(U i = 0; i < 2; ++i)
			{
				Vec4 v0 = unproject(minMax[i], ndc0, projParams);
				Vec4 p0 = move.getPreviousWorldTransform().transform(v0);
				//Vec4 p1 = move.getWorldTransform().transform(p0);
				Vec4 n1p = frc.getViewProjectionMatrix() * p0.xyz1();
				Vec2 n1 = n1p.xy() / n1p.w();
				
				Vec2 t = (n1 / 2.0 + 0.5) * tileCountf;
				t.x() = roundf(t.x());
				t.y() = roundf(t.y());

				if(t.x() < 0.0 || t.x() >= tileCountf.x()
					|| t.y() < 0.0 || t.y() >= tileCountf.y())
				{
					// Outside view. Play it safe
					if(i == 0)
					{
						minMax[i] = getEpsilon<F32>();
					}
					else
					{
						minMax[i] = 1.0 - getEpsilon<F32>();
					}
				}
				else
				{
					// Inside view
					U tileid = U(t.y()) * m_r->getTilesCount().x() + U(t.x());
					minMax[i] = pixels[tileid][i];
				}
			}

			// Calculate the viewspace Z
			minMax = projParams.z() / (projParams.w() + minMax);

			// Calc the planes
			Plane nearPlane = Plane(Vec4(0.0, 0.0, -1.0, 0.0), -minMax.x());
			Plane farPlane = Plane(Vec4(0.0, 0.0, 1.0, 0.0), minMax.y());

			// Tranform them
			m_nearPlanesW[k] = nearPlane.getTransformed(trf);
			m_farPlanesW[k] = farPlane.getTransformed(trf);
		}
	}
}

//==============================================================================
void Tiler::calcPlaneY(U i, const Vec4& projParams)
{
	Plane& plane = m_planesY[i];
	F32 y = F32(i + 1) / m_r->getTilesCount().y() * 2.0 - 1.0;

	Vec4 viewA = unproject(1.0, Vec2(-1.0, y), projParams);
	Vec4 viewB = unproject(1.0, Vec2(1.0, y), projParams);

	Vec4 n = viewB.cross(viewA);
	n.normalize();

	plane = Plane(n, 0.0);
}

//==============================================================================
void Tiler::calcPlaneX(U j, const Vec4& projParams)
{
	Plane& plane = m_planesX[j];
	F32 x = F32(j + 1) / m_r->getTilesCount().x() * 2.0 - 1.0;

	Vec4 viewA = unproject(1.0, Vec2(x, -1.0), projParams);
	Vec4 viewB = unproject(1.0, Vec2(x, 1.0), projParams);

	Vec4 n = viewA.cross(viewB);
	n.normalize();

	plane = Plane(n, 0.0);
}

} // end namespace anki
