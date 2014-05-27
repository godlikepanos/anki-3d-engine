#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ProgramResource.h"
#include "anki/core/Threadpool.h"
#include "anki/scene/Camera.h"
#include <sstream>

// Default should be 0
#define ANKI_TILER_ENABLE_GPU 0

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
#define CHECK_PLANE_PTR(p_) \
	ANKI_ASSERT(U(p_ - &m_tiler->m_allPlanes[0]) < m_tiler->m_allPlanes.size());

/// Job that updates the left, right, top and buttom tile planes
struct UpdatePlanesPerspectiveCameraJob: ThreadpoolTask
{
	Tiler* m_tiler = nullptr;
	PerspectiveCamera* m_cam = nullptr;
	Bool m_frustumChanged;
#if ANKI_TILER_ENABLE_GPU
	const PixelArray* m_pixels = nullptr;
#endif

	void operator()(ThreadId threadId, U threadsCount)
	{
#if ANKI_TILER_ENABLE_GPU
		ANKI_ASSERT(tiler && cam && pixels);
#endif

		PtrSize start, end;
		Transform trf = Transform(m_cam->getWorldTransform());

		if(m_frustumChanged)
		{
			// Re-calculate the planes in local space

			const F32 fx = m_cam->getFovX();
			const F32 fy = m_cam->getFovY();
			const F32 n = m_cam->getNear();

			// Calculate l6 and o6 used to rotate the planes
			F32 l = 2.0 * n * tan(fx / 2.0);
			F32 l6 = l / m_tiler->m_r->getTilesCount().x();
			F32 o = 2.0 * n * tan(fy / 2.0);
			F32 o6 = o / m_tiler->m_r->getTilesCount().y();

			// First the top looking planes
			choseStartEnd(
				threadId, threadsCount, m_tiler->m_r->getTilesCount().y() - 1,
				start, end);

			for(U i = start; i < end; i++)
			{
				calcPlaneI(i, o6);

				CHECK_PLANE_PTR(&m_tiler->m_planesYW[i]);
				CHECK_PLANE_PTR(&m_tiler->m_planesY[i]);
				m_tiler->m_planesYW[i] =
					m_tiler->m_planesY[i].getTransformed(trf);
			}

			// Then the right looking planes
			choseStartEnd(
				threadId, threadsCount, m_tiler->m_r->getTilesCount().x() - 1,
				start, end);

			for(U j = start; j < end; j++)
			{
				calcPlaneJ(j, l6);

				CHECK_PLANE_PTR(&m_tiler->m_planesXW[j]);
				CHECK_PLANE_PTR(&m_tiler->m_planesX[j]);
				m_tiler->m_planesXW[j] =
					m_tiler->m_planesX[j].getTransformed(trf);
			}
		}
		else
		{
			// Only transform planes

			// First the top looking planes
			choseStartEnd(
				threadId, threadsCount, m_tiler->m_r->getTilesCount().y() - 1,
				start, end);

			for(U i = start; i < end; i++)
			{
				CHECK_PLANE_PTR(&m_tiler->m_planesYW[i]);
				CHECK_PLANE_PTR(&m_tiler->m_planesY[i]);
				m_tiler->m_planesYW[i] =
					m_tiler->m_planesY[i].getTransformed(trf);
			}

			// Then the right looking planes
			choseStartEnd(
				threadId, threadsCount, m_tiler->m_r->getTilesCount().x() - 1,
				start, end);

			for(U j = start; j < end; j++)
			{
				CHECK_PLANE_PTR(&m_tiler->m_planesXW[j]);
				CHECK_PLANE_PTR(&m_tiler->m_planesX[j]);
				m_tiler->m_planesXW[j] =
					m_tiler->m_planesX[j].getTransformed(trf);
			}
		}

		// Update the near far planes
#if ANKI_TILER_ENABLE_GPU
		Vec2 rplanes;
		Renderer::calcPlanes(Vec2(cam->getNear(), cam->getFar()), rplanes);

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

	/// Calculate and set a top looking plane
	void calcPlaneI(U i, const F32 o6)
	{
		Vec3 a, b;
		const F32 n = m_cam->getNear();
		Plane& plane = m_tiler->m_planesY[i];
		CHECK_PLANE_PTR(&plane);

		a = Vec3(0.0, 
			(I(i + 1) - I(m_tiler->m_r->getTilesCount().y()) / 2) * o6,
			-n);

		b = Vec3(1.0, 0.0, 0.0).cross(a);
		b.normalize();

		plane = Plane(b, 0.0);
	}

	/// Calculate and set a right looking plane
	void calcPlaneJ(U j, const F32 l6)
	{
		Vec3 a, b;
		const F32 n = m_cam->getNear();
		Plane& plane = m_tiler->m_planesX[j];
		CHECK_PLANE_PTR(&plane);

		a = Vec3((I(j + 1) - I(m_tiler->m_r->getTilesCount().x()) / 2) * l6,
			0.0, 
			-n);

		b = a.cross(Vec3(0.0, 1.0, 0.0));
		b.normalize();

		plane = Plane(b, 0.0);
	}
};

#undef CHECK_PLANE_PTR

//==============================================================================
// Tiler                                                                       =
//==============================================================================

//==============================================================================
Tiler::Tiler()
{}

//==============================================================================
Tiler::~Tiler()
{}

//==============================================================================
void Tiler::init(Renderer* r)
{
	try
	{
		initInternal(r);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init tiler") << e;
	}
}

//==============================================================================
void Tiler::initInternal(Renderer* r)
{
	m_r = r;

	// Load the program
	std::stringstream pps;
	pps << "#define TILES_X_COUNT " << m_r->getTilesCount().x() << "\n"
		<< "#define TILES_Y_COUNT " << m_r->getTilesCount().y() << "\n"
		<< "#define RENDERER_WIDTH " << m_r->getWidth() << "\n"
		<< "#define RENDERER_HEIGHT " << m_r->getHeight() << "\n";

	m_frag.load(ProgramResource::createSrcCodeToCache(
		"shaders/TilerMinMax.frag.glsl", pps.str().c_str(), "r_").c_str());

	m_ppline = m_r->createDrawQuadProgramPipeline(m_frag->getGlProgram());

	// Create FB
	m_r->createRenderTarget(m_r->getTilesCount().x(), m_r->getTilesCount().y(),
		GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 1, m_rt);

	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	m_fb = GlFramebufferHandle(jobs, {{m_rt, GL_COLOR_ATTACHMENT0}});

	// Create PBO
	U pixelBuffSize = m_r->getTilesCount().x() * m_r->getTilesCount().y();
	pixelBuffSize *= 2 * sizeof(F32); // The pixel size
	pixelBuffSize *= 4; // Because it will be always mapped
	m_pixelBuff = GlBufferHandle(jobs, GL_PIXEL_PACK_BUFFER, pixelBuffSize,
		GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	// Init planes. One plane for each direction, plus near/far plus the world
	// space of those
	U planesCount = 
		(r->getTilesCount().x() - 1) * 2 // planes J
		+ (r->getTilesCount().y() - 1) * 2  // planes I
		+ (r->getTilesCount().x() * r->getTilesCount().y() * 2); // near+far

	m_allPlanes.resize(planesCount);

	m_planesX = &m_allPlanes[0];
	m_planesY = m_planesX + m_r->getTilesCount().x() - 1;

	m_planesXW = m_planesY + m_r->getTilesCount().y() - 1;
	m_planesYW = m_planesXW + m_r->getTilesCount().x() - 1;
	m_nearPlanesW = m_planesYW + m_r->getTilesCount().y() - 1;
	m_farPlanesW =
		m_nearPlanesW + m_r->getTilesCount().x() * m_r->getTilesCount().y();

	ANKI_ASSERT(m_farPlanesW + m_r->getTilesCount().x() 
		* m_r->getTilesCount().y() ==  &m_allPlanes[0] + m_allPlanes.size());

	jobs.flush();
}

//==============================================================================
void Tiler::runMinMax(const GlTextureHandle& depthMap)
{
#if ANKI_TILER_ENABLE_GPU
	ANKI_ASSERT(depthMap.getFiltering() == Texture::TFrustumType::NEAREST);

	// Issue the min/max job
	fbo.bind();
	GlStateSingleton::get().setViewport(
		0, 0, r->getTilesCount().x(), r->getTilesCount().y());
	r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
	prog->bind();
	ANKI_ASSERT(depthMapUniform);
	depthMapUniform->set(depthMap);

	r->drawQuad();

	// Issue the async pixel read
	pbo.bind();
	glReadPixels(0, 0, r->getTilesCount().x(), r->getTilesCount().y(), 
		GL_RG_INTEGER, GL_UNSIGNED_INT, nullptr);
	pbo.unbind();
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
	Array<UpdatePlanesPerspectiveCameraJob, Threadpool::MAX_THREADS> jobs;
	U32 camTimestamp = cam.FrustumComponent::getTimestamp();

	// Do a job that transforms only the planes when:
	// - it is the same camera as before and
	// - the camera frustum have not changed
	Bool frustumChanged =
		camTimestamp >= m_planes4UpdateTimestamp || m_prevCam != &cam;

	Threadpool& threadPool = ThreadpoolSingleton::get();

	switch(cam.getCameraType())
	{
	case Camera::Type::PERSPECTIVE:
		for(U i = 0; i < threadPool.getThreadsCount(); i++)
		{
			jobs[i].m_tiler = this;
			jobs[i].m_cam = static_cast<PerspectiveCamera*>(&cam);
#if ANKI_TILER_ENABLE_GPU
			jobs[i].m_pixels = &pixels;
#endif
			jobs[i].m_frustumChanged = frustumChanged;
			threadPool.assignNewTask(i, &jobs[i]);
		}
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	// Update timestamp
	if(frustumChanged)
	{
		m_planes4UpdateTimestamp = getGlobTimestamp();
	}

	// Sync threads
	threadPool.waitForAllThreadsToFinish();

	// 
	// Misc
	// 
	m_prevCam = &cam;
}

//==============================================================================
Bool Tiler::test(
	const CollisionShape& cs, 
	Bool nearPlane,
	Bitset* outBitset) const
{
	Bitset bitset;

	/// Call the recursive function
	testRange(cs, nearPlane, 0, m_r->getTilesCount().y(), 0,
		m_r->getTilesCount().x(), bitset);

	if(outBitset)
	{
		*outBitset = bitset;
	}

	return bitset.any();
}

//==============================================================================
void Tiler::testRange(const CollisionShape& cs, Bool nearPlane,
	U yFrom, U yTo, U xFrom, U xTo, Bitset& bitset) const
{
	U mi = (yTo - yFrom) / 2;
	U mj = (xTo - xFrom) / 2;

	ANKI_ASSERT(mi == mj && "Change the algorithm if they are not the same");

	// Handle final
	if(mi == 0 || mj == 0)
	{
		U tileId = yFrom * m_r->getTilesCount().x() + xFrom;

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

		bitset.set(tileId, inside);
		return;
	}

	// Pick the correct top lookin plane (y)
	const Plane& topPlane = m_planesYW[yFrom + mi - 1];

	// Pick the correct right looking plane (x)
	const Plane& rightPlane = m_planesXW[xFrom + mj - 1];

	// Do the checks
	Bool inside[2][2] = {{false, false}, {false, false}};
	F32 test;

	// Top looking plane check
	test = cs.testPlane(topPlane);
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

	// Right looking plane check
	test = cs.testPlane(rightPlane);
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

	// Now move lower to the hierarchy
	for(U i = 0; i < 2; i++)
	{
		for(U j = 0; j < 2; j++)
		{
			if(inside[i][j])
			{
				testRange(cs, nearPlane,
					yFrom + (i * mi), yFrom + ((i + 1) * mi),
					xFrom + (j * mj), xFrom + ((j + 1) * mj),
					bitset);
			}
		}
	}
}

} // end namespace anki
