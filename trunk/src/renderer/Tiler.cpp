#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/core/ThreadPool.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Spatial.h"

// Default should be 0
#define ANKI_TILER_ENABLE_GPU 0

namespace anki {

//==============================================================================
#define CHECK_PLANE_PTR(p_) \
	ANKI_ASSERT(p_ < &tiler->allPlanes[tiler->allPlanes.size()]);

/// Job that updates the left, right, top and buttom tile planes
struct UpdatePlanesPerspectiveCameraJob: ThreadJob
{
	Tiler* tiler = nullptr;
	PerspectiveCamera* cam = nullptr;
	Bool frustumChanged;
#if ANKI_TILER_ENABLE_GPU
	const Tiler::PixelArray* pixels = nullptr;
#endif

	void operator()(U threadId, U threadsCount)
	{
		ANKI_ASSERT(tiler && cam && pixels);

		U64 start, end;
		Transform trf = Transform(cam->getWorldTransform());

		if(frustumChanged)
		{
			const F32 fx = cam->getFovX();
			const F32 fy = cam->getFovY();
			const F32 n = cam->getNear();

			F32 l = 2.0 * n * tan(fx / 2.0);
			F32 l6 = l / Tiler::TILES_X_COUNT;
			F32 o = 2.0 * n * tan(fy / 2.0);
			F32 o6 = o / Tiler::TILES_Y_COUNT;

			// First the top looking planes
			choseStartEnd(
				threadId, threadsCount, Tiler::TILES_Y_COUNT - 1, start, end);

			for(U i = start; i < end; i++)
			{
				calcPlaneI(i, o6);

				CHECK_PLANE_PTR(&tiler->planesIW[i]);
				CHECK_PLANE_PTR(&tiler->planesI[i]);
				tiler->planesIW[i] = tiler->planesI[i].getTransformed(trf);
			}

			// Then the right looking planes
			choseStartEnd(
				threadId, threadsCount, Tiler::TILES_X_COUNT - 1, start, end);

			for(U j = start; j < end; j++)
			{
				calcPlaneJ(j, l6);

				CHECK_PLANE_PTR(&tiler->planesJW[j]);
				CHECK_PLANE_PTR(&tiler->planesJ[j]);
				tiler->planesJW[j] = tiler->planesJ[j].getTransformed(trf);
			}
		}
		else
		{
			// Only transform planes

			// First the top looking planes
			choseStartEnd(
				threadId, threadsCount, Tiler::TILES_Y_COUNT - 1, start, end);

			for(U i = start; i < end; i++)
			{
				CHECK_PLANE_PTR(&tiler->planesIW[i]);
				CHECK_PLANE_PTR(&tiler->planesI[i]);
				tiler->planesIW[i] = tiler->planesI[i].getTransformed(trf);
			}

			// Then the right looking planes
			choseStartEnd(
				threadId, threadsCount, Tiler::TILES_X_COUNT - 1, start, end);

			for(U j = start; j < end; j++)
			{
				CHECK_PLANE_PTR(&tiler->planesJW[j]);
				CHECK_PLANE_PTR(&tiler->planesJ[j]);
				tiler->planesJW[j] = tiler->planesJ[j].getTransformed(trf);
			}
		}

		// Update the near far planes
#if ANKI_TILER_ENABLE_GPU
		Vec2 rplanes;
		Renderer::calcPlanes(Vec2(cam->getNear(), cam->getFar()), rplanes);

		choseStartEnd(
			threadId, threadsCount, Tiler::TILES_COUNT, start, end);

		Plane* nearPlanesW = tiler->nearPlanesW + start;
		Plane* farPlanesW = tiler->farPlanesW + start;
		for(U k = start; k < end; ++k)
		{
			U j = k % Tiler::TILES_X_COUNT;
			U i = k / Tiler::TILES_X_COUNT;

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
		const F32 n = cam->getNear();
		Plane& plane = tiler->planesI[i];
		CHECK_PLANE_PTR(&plane);

		a = Vec3(0.0, (I(i + 1) - I(Tiler::TILES_Y_COUNT) / 2) * o6, -n);
		b = Vec3(1.0, 0.0, 0.0).cross(a);
		b.normalize();

		plane = Plane(b, 0.0);
	}

	/// Calculate and set a right looking plane
	void calcPlaneJ(U j, const F32 l6)
	{
		Vec3 a, b;
		const F32 n = cam->getNear();
		Plane& plane = tiler->planesJ[j];
		CHECK_PLANE_PTR(&plane);

		a = Vec3((I(j + 1) - I(Tiler::TILES_X_COUNT) / 2) * l6, 0.0, -n);
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
void Tiler::init(Renderer* r_)
{
	try
	{
		initInternal(r_);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init tiler") << e;
	}
}

//==============================================================================
void Tiler::initInternal(Renderer* r_)
{
	r = r_;

	// Load the program
	std::string pps =
		"#define TILES_X_COUNT " + std::to_string(TILES_X_COUNT) + "\n"
		"#define TILES_Y_COUNT " + std::to_string(TILES_Y_COUNT) + "\n"
		"#define RENDERER_WIDTH " + std::to_string(r->getWidth()) + "\n"
		"#define RENDERER_HEIGHT " + std::to_string(r->getHeight()) + "\n";

	prog.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/TilerMinMax.glsl", pps.c_str()).c_str());

	depthMapUniform = &(prog->findUniformVariable("depthMap"));

	// Create FBO
	Renderer::createFai(TILES_X_COUNT, TILES_Y_COUNT, GL_RG32UI,
		GL_RG_INTEGER, GL_UNSIGNED_INT, fai);
	fai.setFiltering(Texture::TFT_NEAREST);

	fbo.create();
	fbo.setColorAttachments({&fai});
	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("FBO not complete");
	}

	// Create PBO
	pbo.create(GL_PIXEL_PACK_BUFFER, 
		TILES_X_COUNT * TILES_Y_COUNT * 2 * sizeof(F32), nullptr);

	// Init planes
	U planesCount = 
		(TILES_X_COUNT - 1) * 2 // planes J
		+ (TILES_Y_COUNT - 1) * 2  // planes I
		+ (TILES_COUNT * 2); // near far planes

	allPlanes.resize(planesCount);

	planesJ = &allPlanes[0];
	planesI = planesJ + TILES_X_COUNT - 1;

	planesJW = planesI + TILES_Y_COUNT - 1;
	planesIW = planesJW + TILES_X_COUNT - 1;
	nearPlanesW = planesIW + TILES_Y_COUNT - 1;
	farPlanesW = nearPlanesW + TILES_COUNT;
}

//==============================================================================
void Tiler::runMinMax(const Texture& depthMap)
{
#if ANKI_TILER_ENABLE_GPU
	ANKI_ASSERT(depthMap.getFiltering() == Texture::TFT_NEAREST);

	// Issue the min/max job
	fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, TILES_X_COUNT, TILES_Y_COUNT);
	r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
	prog->bind();
	ANKI_ASSERT(depthMapUniform);
	depthMapUniform->set(depthMap);

	r->drawQuad();

	// Issue the async pixel read
	pbo.bind();
	glReadPixels(0, 0, TILES_X_COUNT, TILES_Y_COUNT, GL_RG_INTEGER,
		GL_UNSIGNED_INT, nullptr);
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
	Array<UpdatePlanesPerspectiveCameraJob, ThreadPool::MAX_THREADS> jobs;
	U32 camTimestamp = cam.getFrustumable()->getFrustumableTimestamp();

	// Transform only the planes when:
	// - it is the same camera as before and
	// - the camera frustum have not changed
	Bool frustumChanged =
		camTimestamp >= planes4UpdateTimestamp || prevCam != &cam;

	ThreadPool& threadPool = ThreadPoolSingleton::get();

	switch(cam.getCameraType())
	{
	case Camera::CT_PERSPECTIVE:
		for(U i = 0; i < threadPool.getThreadsCount(); i++)
		{
			jobs[i].tiler = this;
			jobs[i].cam = static_cast<PerspectiveCamera*>(&cam);
#if ANKI_TILER_ENABLE_GPU
			jobs[i].pixels = &pixels;
#endif
			jobs[i].frustumChanged = frustumChanged;
			threadPool.assignNewJob(i, &jobs[i]);
		}
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	if(frustumChanged)
	{
		planes4UpdateTimestamp = Timestamp::getTimestamp();
	}

	threadPool.waitForAllJobsToFinish();

	// 
	// Misc
	// 
	prevCam = &cam;
}

//==============================================================================
Bool Tiler::test2(
	const CollisionShape& cs, 
	const Aabb& aabb, 
	Bool nearPlane,
	Bitset* outBitset) const
{
	Bitset bitset;

	testRange(cs, nearPlane, 0, TILES_Y_COUNT, 0, TILES_X_COUNT, bitset);

	if(outBitset)
	{
		*outBitset = bitset;
	}

	return bitset.any();
}

//==============================================================================
void Tiler::testRange(const CollisionShape& cs, Bool nearPlane,
	U iFrom, U iTo, U jFrom, U jTo, Bitset& bitset) const
{
	U mi = (iTo - iFrom) / 2;
	U mj = (jTo - jFrom) / 2;

	ANKI_ASSERT(mi == mj && "Change the algorithm if they are not the same");

	// Handle final
	if(mi == 0 || mj == 0)
	{
		U tileId = iFrom * TILES_X_COUNT + jFrom;

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

	// Find the correct top lookin plane (i)
	const Plane& topPlane = planesIW[iFrom + mi - 1];

	// Find the correct right looking plane (j)
	const Plane& rightPlane = planesJW[jFrom + mj - 1];

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
					iFrom + (i * mi), iFrom + ((i + 1) * mi),
					jFrom + (j * mj), jFrom + ((j + 1) * mj),
					bitset);
			}
		}
	}
}

} // end namespace anki
