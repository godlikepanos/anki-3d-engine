// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ProgramResource.h"
#include "anki/scene/Camera.h"

// Default should be 0
#define ANKI_TILER_ENABLE_GPU 0

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
#define CHECK_PLANE_PTR(p_) ANKI_ASSERT( \
	U(p_ - &m_allPlanes[0]) < m_allPlanes.getSize());

//==============================================================================
class UpdatePlanesPerspectiveCameraTask: public Threadpool::Task
{
public:
	Tiler* m_tiler = nullptr;
	Camera* m_cam = nullptr;
	Bool m_frustumChanged;
#if ANKI_TILER_ENABLE_GPU
	const PixelArray* m_pixels = nullptr;
#endif

	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		m_tiler->update(threadId, threadsCount, *m_cam, m_frustumChanged);

		return ErrorCode::NONE;
	}
};

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
	Error err = ErrorCode::NONE;

	// Load the program
	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());

	err = pps.sprintf(getAllocator(),
		"#define TILES_X_COUNT %u\n"
		"#define TILES_Y_COUNT %u\n"
		"#define RENDERER_WIDTH %u\n"
		"#define RENDERER_HEIGHT %u\n",
		m_r->getTilesCount().x(),
		m_r->getTilesCount().y(),
		m_r->getWidth(),
		m_r->getHeight());
	if(err) return err;

	err = m_frag.loadToCache(&getResourceManager(),
		"shaders/TilerMinMax.frag.glsl", pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadPipeline(m_frag->getGlProgram(), m_ppline);
	if(err) return err;

	// Create FB
	err = m_r->createRenderTarget(
		m_r->getTilesCount().x(), m_r->getTilesCount().y(), GL_RG32UI, 1, m_rt);
	if(err) return err;

	GlCommandBufferHandle cmdBuff;
	err = cmdBuff.create(&getGlDevice());
	if(err) return err;

	err = m_fb.create(cmdBuff, {{m_rt, GL_COLOR_ATTACHMENT0}});
	if(err) return err;

	// Create PBO
	U pixelBuffSize = m_r->getTilesCount().x() * m_r->getTilesCount().y();
	pixelBuffSize *= 2 * sizeof(F32); // The pixel size
	pixelBuffSize *= 4; // Because it will be always mapped
	err = m_pixelBuff.create(cmdBuff, GL_PIXEL_PACK_BUFFER, pixelBuffSize,
		GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	if(err) return err;

	// Init planes. One plane for each direction, plus near/far plus the world
	// space of those
	U planesCount = 
		(m_r->getTilesCount().x() - 1) * 2 // planes J
		+ (m_r->getTilesCount().y() - 1) * 2  // planes I
		+ (m_r->getTilesCount().x() * m_r->getTilesCount().y() * 2); // near+far

	err = m_allPlanes.create(getAllocator(), planesCount);
	if(err) return err;

	m_planesX = &m_allPlanes[0];
	m_planesY = m_planesX + m_r->getTilesCount().x() - 1;

	m_planesXW = m_planesY + m_r->getTilesCount().y() - 1;
	m_planesYW = m_planesXW + m_r->getTilesCount().x() - 1;
	m_nearPlanesW = m_planesYW + m_r->getTilesCount().y() - 1;
	m_farPlanesW =
		m_nearPlanesW + m_r->getTilesCount().x() * m_r->getTilesCount().y();

	ANKI_ASSERT(m_farPlanesW + m_r->getTilesCount().x() 
		* m_r->getTilesCount().y() ==  &m_allPlanes[0] + m_allPlanes.getSize());

	cmdBuff.flush();

	return err;
}

//==============================================================================
void Tiler::runMinMax(GlTextureHandle& depthMap,
	GlCommandBufferHandle& cmd)
{
#if ANKI_TILER_ENABLE_GPU
	// Issue the min/max job
	m_fb.bind(cmd, true);
	m_ppline.bind(cmd);
	depthMap.bind(cmd, 0);

	m_r->drawQuad(cmd);

	// Issue the async pixel read
	cmd.copyTextureToBuffer(m_rt, m_pixelBuff);
#endif
}

//==============================================================================
void Tiler::updateTiles(Camera& cam)
{
	//
	// Read the results from the minmax job. It will block
	//
#if ANKI_TILER_ENABLE_GPU
	PixelArray pixels;
	pbo.read(&pixels[0][0]);
#endif

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
#if ANKI_TILER_ENABLE_GPU
		jobs[i].m_pixels = &pixels;
#endif
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
Bool Tiler::test(
	const CollisionShape& cs, 
	Bool nearPlane,
	VisibleTiles* visible) const
{
	if(visible)
	{
		visible->m_count = 0;
	}

	// Call the recursive function
	U count = 0;
	testRange(cs, nearPlane, 0, m_r->getTilesCount().y(), 0,
		m_r->getTilesCount().x(), visible, count);

	if(visible)
	{
		ANKI_ASSERT(count == visible->m_count);
	}

	return count > 0;
}

//==============================================================================
void Tiler::testRange(const CollisionShape& cs, Bool nearPlane,
	U yFrom, U yTo, U xFrom, U xTo, VisibleTiles* visible, U& count) const
{
	U my = (yTo - yFrom) / 2;
	U mx = (xTo - xFrom) / 2;

	// Handle final
	if(ANKI_UNLIKELY(my == 0 && mx == 0))
	{
		Bool inside = true;

#if ANKI_TILER_ENABLE_GPU
		if(cs.testPlane(farPlanesW[tileId]) >= 0.0)
		{
			// Inside on far but check near

			if(nearPlane)
			{
				if(cs.testPlane(nearPlanesW[tileId]) >= 0)
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
#endif

		if(inside)
		{
			if(visible)
			{
				VisibleTiles::Pair p;
				ANKI_ASSERT(xFrom < 0xFF && yFrom < 0xFF);
				p.m_x = static_cast<U8>(xFrom);
				p.m_y = static_cast<U8>(yFrom);
				visible->m_tileIds[visible->m_count++] = p;
			}

			++count;
		}

		return;
	}

	// Do the checks
	Bool inside[2][2] = {{false, false}, {false, false}};

	// Top looking plane check
	if(my > 0)
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

	// Right looking plane check
	if(mx > 0)
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
	if(mx == 0)
	{
		if(inside[0][0])
		{
			testRange(cs, nearPlane,
				yFrom, yFrom + my,
				xFrom, xTo,
				visible, count);
		}

		if(inside[1][0])
		{
			testRange(cs, nearPlane,
				yFrom + my, yTo,
				xFrom, xTo,
				visible, count);
		}
	}
	else if(my == 0)
	{
		if(inside[0][0])
		{
			testRange(cs, nearPlane,
				yFrom, yTo,
				xFrom, xFrom + mx,
				visible, count);
		}

		if(inside[0][1])
		{
			testRange(cs, nearPlane,
				yFrom, yTo,
				xFrom + mx, xTo,
				visible, count);
		}
	}
	else
	{
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
}

//==============================================================================
void Tiler::update(U32 threadId, PtrSize threadsCount, 
	Camera& cam, Bool frustumChanged)
{
	PtrSize start, end;
	const MoveComponent& move = cam.getComponent<MoveComponent>();
	const FrustumComponent& fr = cam.getComponent<FrustumComponent>();
	ANKI_ASSERT(fr.getFrustum().getType() == Frustum::Type::PERSPECTIVE);
	const PerspectiveFrustum& frustum = 
		static_cast<const PerspectiveFrustum&>(fr.getFrustum());

	const Transform& trf = move.getWorldTransform();

	if(frustumChanged)
	{
		// Re-calculate the planes in local space

		const F32 fx = frustum.getFovX();
		const F32 fy = frustum.getFovY();
		const F32 n = frustum.getNear();

		// Calculate l6 and o6 used to rotate the planes
		F32 l = 2.0 * n * tan(fx / 2.0);
		F32 l6 = l / m_r->getTilesCount().x();
		F32 o = 2.0 * n * tan(fy / 2.0);
		F32 o6 = o / m_r->getTilesCount().y();

		// First the top looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().y() - 1,
			start, end);

		for(U i = start; i < end; i++)
		{
			calcPlaneY(i, o6, n);

			CHECK_PLANE_PTR(&m_planesYW[i]);
			CHECK_PLANE_PTR(&m_planesY[i]);
			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().x() - 1,
			start, end);

		for(U j = start; j < end; j++)
		{
			calcPlaneX(j, l6, n);

			CHECK_PLANE_PTR(&m_planesXW[j]);
			CHECK_PLANE_PTR(&m_planesX[j]);
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
			CHECK_PLANE_PTR(&m_planesYW[i]);
			CHECK_PLANE_PTR(&m_planesY[i]);
			m_planesYW[i] = m_planesY[i].getTransformed(trf);
		}

		// Then the right looking planes
		Threadpool::Task::choseStartEnd(
			threadId, threadsCount, m_r->getTilesCount().x() - 1,
			start, end);

		for(U j = start; j < end; j++)
		{
			CHECK_PLANE_PTR(&m_planesXW[j]);
			CHECK_PLANE_PTR(&m_planesX[j]);
			m_planesXW[j] = m_planesX[j].getTransformed(trf);
		}
	}

	// Update the near far planes
#if ANKI_TILER_ENABLE_GPU
	Vec2 rplanes;
	Renderer::calcPlanes(Vec2(cam.getNear(), cam.getFar()), rplanes);

	choseStartEnd(
		threadId, threadsCount, 
		tiler->r->getTilesCount().x() * tiler->r->getTilesCount().y(), 
		start, end);

	Plane* nearPlanesW = tiler->nearPlanesW + start;
	Plane* farPlanesW = tiler->farPlanesW + start;
	for(U k = start; k < end; ++k)
	{
		U j = k % tiler->r->getTilesCount().x();
		U i = k / tiler->r->getTilesCount().x();

		// Calculate depth as you do it for the vertex position inside
		// the shaders
		F32 minZ = rplanes.y() / (rplanes.x() + (*pixels)[i][j][0]);
		F32 maxZ = rplanes.y() / (rplanes.x() + (*pixels)[i][j][1]);

		// Calc the planes
		Plane nearPlane = Plane(Vec3(0.0, 0.0, -1.0), minZ);
		Plane farPlane = Plane(Vec3(0.0, 0.0, 1.0), -maxZ);

		// Tranform them
		CHECK_PLANE_PTR(nearPlanesW);
		*nearPlanesW = nearPlane.getTransformed(trf);
		CHECK_PLANE_PTR(farPlanesW);
		*farPlanesW = farPlane.getTransformed(trf);

		// Advance
		++nearPlanesW;
		++farPlanesW;
	}
#endif
}

//==============================================================================
void Tiler::calcPlaneY(U i, const F32 o6, const F32 near)
{
	Vec4 a, b;
	Plane& plane = m_planesY[i];
	ANKI_ASSERT(i < m_allPlanes.getSize());

	a = Vec4(0.0, 
		(I(i + 1) - I(m_r->getTilesCount().y()) / 2) * o6,
		-near,
		0.0);

	b = Vec4(1.0, 0.0, 0.0, 0.0).cross(a);
	b.normalize();

	plane = Plane(b, 0.0);
}

//==============================================================================
void Tiler::calcPlaneX(U j, const F32 l6, const F32 near)
{
	Vec4 a, b;
	Plane& plane = m_planesX[j];
	ANKI_ASSERT(j < m_allPlanes.getSize());

	a = Vec4((I(j + 1) - I(m_r->getTilesCount().x()) / 2) * l6,
		0.0, 
		-near,
		0.0);

	b = a.cross(Vec4(0.0, 1.0, 0.0, 0.0));
	b.normalize();

	plane = Plane(b, 0.0);
}

} // end namespace anki
