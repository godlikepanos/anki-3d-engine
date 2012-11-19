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
struct UpdateTiles4PlanesPerspectiveCameraJob: ThreadJob
{
	Tiler* tiler;
	PerspectiveCamera* cam;
	Bool onlyTransformPlanes;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, 
			Tiler::TILES_X_COUNT * Tiler::TILES_Y_COUNT, start, end);

		const F32 fx = cam->getFovX();
		const F32 fy = cam->getFovY();
		const F32 n = cam->getNear();

		F32 l = 2.0 * n * tan(fx / 2.0);
		F32 l6 = l / Tiler::TILES_X_COUNT;
		F32 o = 2.0 * n * tan(fy / 2.0);
		F32 o6 = o / Tiler::TILES_Y_COUNT;

		Transform trf = Transform(cam->getWorldTransform());

		for(U64 k = start; k < end; k++)
		{
			if(!onlyTransformPlanes)
			{
				updatePlanes(l, l6, o, o6, k);
			}

			Tiler::Tile& tile = tiler->tiles1d[k];

			tile.planesWSpace[Frustum::FP_LEFT] = 
				tile.planes[Frustum::FP_LEFT].getTransformed(trf);
			tile.planesWSpace[Frustum::FP_RIGHT] = 
				tile.planes[Frustum::FP_RIGHT].getTransformed(trf);
			tile.planesWSpace[Frustum::FP_BOTTOM] = 
				tile.planes[Frustum::FP_BOTTOM].getTransformed(trf);
			tile.planesWSpace[Frustum::FP_TOP] = 
				tile.planes[Frustum::FP_TOP].getTransformed(trf);
		}
	}

	void updatePlanes(
		const F32 l, const F32 l6, const F32 o, const F32 o6,
		const U k)
	{
		Vec3 a, b;
		const F32 n = cam->getNear();
		const U i = k % Tiler::TILES_X_COUNT;
		const U j = k / Tiler::TILES_X_COUNT;
		Array<Plane, Frustum::FP_COUNT>& planes = tiler->tiles1d[k].planes;

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
};

typedef Array<UpdateTiles4PlanesPerspectiveCameraJob, ThreadPool::MAX_THREADS>
	Update4JobArray;

//==============================================================================
/// Job that updates the near and far tile planes and transforms them
struct UpdateTiles2PlanesJob: ThreadJob
{
	const Tiler::PixelArray* pixels;
	Tiler* tiler;
	Camera* cam;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, 
			Tiler::TILES_X_COUNT * Tiler::TILES_Y_COUNT, start, end);

		// Calc planes for this camera
		Vec2 planes;
		Renderer::calcPlanes(Vec2(cam->getNear(), cam->getFar()), planes);
		Transform trf = Transform(cam->getWorldTransform());

		for(U64 k = start; k < end; k++)
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

			// Transform planes
			tile.planesWSpace[Frustum::FP_NEAR] = 
				tile.planes[Frustum::FP_NEAR].getTransformed(trf);
			tile.planesWSpace[Frustum::FP_FAR] = 
				tile.planes[Frustum::FP_FAR].getTransformed(trf);
		}
	}
};

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
}

//==============================================================================
void Tiler::updateTiles(Camera& cam, const Texture& depthMap)
{
	Update4JobArray jobs4;

#if !ALTERNATIVE_MIN_MAX
	//
	// Issue the min/max draw call
	//

	update4Planes(cam, &jobs4);

	fbo.bind(); // Flush prev FBO to force flush on Mali
	GlStateSingleton::get().setViewport(0, 0, TILES_X_COUNT, TILES_Y_COUNT);
	prog->bind();
	prog->findUniformVariable("depthMap").set(depthMap);

	r->drawQuad();

	waitUpdate4Planes();

	//
	// Read pixels from the min/max pass
	//

	PixelArray pixels;
#if ANKI_GL == ANKI_GL_DESKTOP
	// It seems read from texture is a bit faster than readpixels on nVidia
	fai.readPixels(pixels);
#else
	glReadPixels(0, 0, TILES_X_COUNT, TILES_Y_COUNT, GL_RG_INTEGER,
		GL_UNSIGNED_INT, pixels);
#endif

	// 
	// Update and transform the 2 planes
	// 
	update2Planes(cam, pixels);

	prevCam = &cam;
#else
	Vector<F32> allpixels;

	update4Planes(cam, &jobs4);

	allpixels.resize(r->getWidth() * r->getHeight());
	glReadPixels(0, 0, r->getWidth(), r->getHeight(), GL_DEPTH_COMPONENT,
		GL_FLOAT, &allpixels[0]);

	// Init pixels
	PixelArray pixels;
	for(U j = 0; j < TILES_Y_COUNT; j++)
	{
		for(U i = 0; i < TILES_X_COUNT; i++)
		{
			pixels[j][i][0] = 1.0;
			pixels[j][i][1] = 0.0;
		}
	}

	// Get min max
	U w = r->getWidth() / TILES_X_COUNT;
	U h = r->getHeight() / TILES_Y_COUNT;
	for(U j = 0; j < r->getHeight(); j++)
	{
		for(U i = 0; i < r->getWidth(); i++)
		{
			U ii = i / w;
			U jj = j / h;

			F32 depth = allpixels[j * r->getWidth() + i];

			pixels[jj][ii][0] = std::min(pixels[jj][ii][0], depth);
			pixels[jj][ii][1] = std::max(pixels[jj][ii][1], depth);
		}
	}

	waitUpdate4Planes();

	// Update and transform the 2 planes
	update2Planes(cam, pixels);

	prevCam = &cam;

#endif
}

//==============================================================================
void Tiler::update4Planes(Camera& cam, void* jobs_)
{
	Update4JobArray& jobs = *(Update4JobArray*)jobs_;

	U32 camTimestamp = cam.getFrustumable()->getFrustumableTimestamp();

	// Transform only the planes when:
	// - it is the same camera as before and
	// - the camera frustum have not changed
	Bool onlyTransformPlanes = 
		camTimestamp < planes4UpdateTimestamp && prevCam == &cam;

	// Update the planes in parallel
	// 
	ThreadPool& threadPool = ThreadPoolSingleton::get();

	switch(cam.getCameraType())
	{
	case Camera::CT_PERSPECTIVE:
		for(U i = 0; i < threadPool.getThreadsCount(); i++)
		{	
			jobs[i].tiler = this;
			jobs[i].cam = static_cast<PerspectiveCamera*>(&cam);
			jobs[i].onlyTransformPlanes = onlyTransformPlanes;
			threadPool.assignNewJob(i, &jobs[i]);
		}
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	if(!onlyTransformPlanes)
	{
		planes4UpdateTimestamp = Timestamp::getTimestamp();
	}
}

//==============================================================================
void Tiler::waitUpdate4Planes()
{
	ThreadPoolSingleton::get().waitForAllJobsToFinish();
}

//==============================================================================
void Tiler::update2Planes(Camera& cam, const PixelArray& pixels)
{
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	UpdateTiles2PlanesJob jobs[ThreadPool::MAX_THREADS];
	
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].pixels = &pixels;
		jobs[i].tiler = this;
		jobs[i].cam = &cam;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
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
