#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/core/ThreadPool.h"
#include "anki/scene/Camera.h"

// Default should be 0
#define ALTERNATIVE_MIN_MAX 0

namespace anki {

//==============================================================================
/// Job that updates the left, right, top and buttom tile planes
struct UpdateTilesPlanesPerspectiveCameraJob: ThreadJob
{
	Tiler* tiler;
	PerspectiveCamera* cam;
	Bool frustumChanged;
	const Tiler::PixelArray* pixels;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, 
			Tiler::TILES_X_COUNT * Tiler::TILES_Y_COUNT, start, end);

		// Precalculate some stuff for update4Planes()
		F32 l = 0.0, l6 = 0.0, o = 0.0, o6 = 0.0;

		if(frustumChanged)
		{
			const F32 fx = cam->getFovX();
			const F32 fy = cam->getFovY();
			const F32 n = cam->getNear();

			l = 2.0 * n * tan(fx / 2.0);
			l6 = l / Tiler::TILES_X_COUNT;
			o = 2.0 * n * tan(fy / 2.0);
			o6 = o / Tiler::TILES_Y_COUNT;
		}

		// Precalculate some stuff for update2Planes()
		Vec2 planes;
		Renderer::calcPlanes(Vec2(cam->getNear(), cam->getFar()), planes);

		Transform trf = Transform(cam->getWorldTransform());

		for(U64 k = start; k < end; k++)
		{
			if(frustumChanged)
			{
				update4Planes(l, l6, o, o6, k);
			}

			update2Planes(k, planes);

			// Transform planes
			Tiler::Tile& tile = tiler->tiles1d[k];
			for(U i = 0; i < Frustum::FP_COUNT; i++)
			{
				tile.planesWSpace[i] = tile.planes[i].getTransformed(trf);
			}
		}
	}

	void update4Planes(
		const F32 l, const F32 l6, const F32 o, const F32 o6,
		const U k)
	{
		Vec3 a, b;
		const F32 n = cam->getNear();
		Array<Plane, Frustum::FP_COUNT>& planes = tiler->tiles1d[k].planes;
		const U i = k % Tiler::TILES_X_COUNT;
		const U j = k / Tiler::TILES_X_COUNT;

		// left
		a = Vec3((I(i) - I(Tiler::TILES_X_COUNT) / 2) * l6, 0.0, -n);
		b = a.cross(Vec3(0.0, 1.0, 0.0));
		b.normalize();

		planes[Frustum::FP_LEFT] = Plane(b, 0.0);

		// right
		a = Vec3((I(i) - I(Tiler::TILES_X_COUNT) / 2 + 1) * l6, 0.0, -n);
		b = Vec3(0.0, 1.0, 0.0).cross(a);
		b.normalize();

		planes[Frustum::FP_RIGHT] = Plane(b, 0.0);

		// bottom
		a = Vec3(0.0, (I(j) - I(Tiler::TILES_Y_COUNT) / 2) * o6, -n);
		b = Vec3(1.0, 0.0, 0.0).cross(a);
		b.normalize();

		planes[Frustum::FP_BOTTOM] = Plane(b, 0.0);

		// bottom
		a = Vec3(0.0, (I(j) - I(Tiler::TILES_Y_COUNT) / 2 + 1) * o6, -n);
		b = a.cross(Vec3(1.0, 0.0, 0.0));
		b.normalize();

		planes[Frustum::FP_TOP] = Plane(b, 0.0);
	}

	void update2Planes(const U k, const Vec2& planes)
	{
		U i = k % Tiler::TILES_X_COUNT;
		U j = k / Tiler::TILES_X_COUNT;
		Tiler::Tile& tile = tiler->tiles1d[k];

		// Calculate depth as you do it for the vertex position inside
		// the shaders
		F32 minZ = planes.y() / (planes.x() + (*pixels)[j][i][0]);
		F32 maxZ = planes.y() / (planes.x() + (*pixels)[j][i][1]);

		tile.planes[Frustum::FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), minZ);
		tile.planes[Frustum::FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), -maxZ);
	}
};

typedef Array<UpdateTilesPlanesPerspectiveCameraJob, ThreadPool::MAX_THREADS>
	UpdateJobArray;

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
}

//==============================================================================
void Tiler::runMinMax(const Texture& depthMap)
{
	// Issue the min/max job
	fbo.bind();
	GlStateSingleton::get().setViewport(0, 0, TILES_X_COUNT, TILES_Y_COUNT);
	r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
	prog->bind();
	depthMapUniform->set(depthMap);

	r->drawQuad();

	// Issue the async pixel read
	pbo.bind();
	glReadPixels(0, 0, TILES_X_COUNT, TILES_Y_COUNT, GL_RG_INTEGER,
		GL_UNSIGNED_INT, nullptr);
	pbo.unbind();
}

//==============================================================================
void Tiler::updateTiles(Camera& cam)
{
	//
	// Read the results from the minmax job
	//
	PixelArray pixels;
	pbo.read(&pixels[0][0]);

	//
	// Issue parallel jobs
	//
	UpdateJobArray jobs;

	U32 camTimestamp = cam.getFrustumable()->getFrustumableTimestamp();

	// Transform only the planes when:
	// - it is the same camera as before and
	// - the camera frustum have not changed
	Bool update4Planes =
		camTimestamp >= planes4UpdateTimestamp || prevCam != &cam;

	// Update the planes in parallel
	//
	ThreadPool& threadPool = ThreadPoolSingleton::get();

	switch(cam.getCameraType())
	{
	case Camera::CT_PERSPECTIVE:
		for(U i = 0; i < threadPool.getThreadsCount(); i++)
		{
			jobs[i].pixels = &pixels;
			jobs[i].tiler = this;
			jobs[i].cam = static_cast<PerspectiveCamera*>(&cam);
			jobs[i].frustumChanged = update4Planes;
			threadPool.assignNewJob(i, &jobs[i]);
		}
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	if(update4Planes)
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
Bool Tiler::testInternal(const CollisionShape& cs, const Tile& tile, 
	const U startPlane) const
{
	for(U j = startPlane; j < Frustum::FP_COUNT; j++)
	{
		if(cs.testPlane(tile.planesWSpace[j]) < 0.0)
		{
			return false;
		}
	}
	return true;
}

//==============================================================================
Bool Tiler::test(const CollisionShape& cs, const U32 tileId,
	const Bool skipNearPlaneCheck) const
{
	return testInternal(cs, tiles1d[tileId], (skipNearPlaneCheck) ? 1 : 0);
}

//==============================================================================
Bool Tiler::testAll(const CollisionShape& cs, 
	const Bool skipNearPlaneCheck) const
{
	static_assert(Frustum::FP_NEAR == 0, "Frustum::FP_NEAR should be 0");

	U startPlane = (skipNearPlaneCheck) ? 1 : 0;

	for(const Tile& tile : tiles1d)
	{
		if(testInternal(cs, tile, startPlane))
		{
			return true;
		}
	}

	return false;
}

} // end namespace anki
