#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/core/ThreadPool.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
/// Job that updates the left, right, top and buttom tile planes
struct UpdateTiles4PlanesPerspectiveCameraJob: ThreadJob
{
	Tiler* tiler;
	PerspectiveCamera* cam;

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

		for(U64 k = start; k < end; k++)
		{
			U i = k % Tiler::TILES_X_COUNT;
			U j = k / Tiler::TILES_X_COUNT;

			Array<Plane, Frustum::FP_COUNT>& planes = tiler->tiles[j][i].planes;
			updatePlanes(l, l6, o, o6, i, j, planes);
		}
	}

	void updatePlanes(
		const F32 l, const F32 l6, const F32 o, const F32 o6,
		const U i, const U j,
		Array<Plane, Frustum::FP_COUNT>& planes)
	{
		Vec3 a, b;
		const F32 n = cam->getNear();

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

//==============================================================================
/// Job that updates the near and far tile planes and transforms them
struct UpdateTiles2PlanesJob: ThreadJob
{
	F32 (*pixels)[Is::TILES_Y_COUNT][Is::TILES_X_COUNT][2];
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

		for(U64 k = start; k < end; k++)
		{
			U i = k % Tiler::TILES_X_COUNT;
			U j = k / Tiler::TILES_X_COUNT;
			Tiler::Tile& tile = tiler->tiles[j][i];

			// Calculate depth as you do it for the vertex position inside 
			// the shaders
			F32 minZ = planes.y() / (planes.x() + (*pixels)[j][i][0]);
			F32 maxZ = -planes.y() / (planes.x() + (*pixels)[j][i][1]);

			tile.planes[Frustum::FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), minZ);
			tile.planes[Frustum::FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), maxZ);

			// Transform all planes
			Transform trf = Transform(cam->getWorldTransform());
			for(U k = 0; k < Frustum::FP_COUNT; k++)
			{
				tile.planesWSpace[k] = tile.planes[k].getTransformed(trf);
			}
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
		"shaders/IsMinMax.glsl", pps.c_str()).c_str());

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
	//
	// In the meantime update the 4 planes for all the tiles
	//
	fbo.bind(); // Flush prev FBO on Mali
	update4Planes(cam);

	//
	// Issue the min/max draw call
	//
	prog->bind();
	GlStateSingleton::get().setViewport(0, 0, TILES_X_COUNT, TILES_Y_COUNT);
	prog->findUniformVariable("depthMap").set(depthMap);

	r->drawQuad();

#if ANKI_GL == ANKI_GL_ES
	// For Mali T6xx do a flush because the GPU will stay doing nothing until 
	// read pixels later on where it will be forced to flush
	glFlush();
#endif

	//
	// Read pixels from the min/max pass
	//

	F32 pixels[TILES_Y_COUNT][TILES_X_COUNT][2];
#if ANKI_GL == ANKI_GL_DESKTOP
	// It seems read from texture is a bit faster than readpixels on nVidia
	fai.readPixels(pixels);
#else
	glReadPixels(0, 0, TILES_X_COUNT, TILES_Y_COUNT, GL_RG_INTEGER,
		GL_UNSIGNED_INT, &pixels[0][0][0]);
#endif

	// 
	// Update the 2 planes and transform the planes
	// 
	update2Planes(cam, &pixels);

	prevCam = &cam;
}

//==============================================================================
void Tiler::update4Planes(Camera& cam)
{
	U32 camTimestamp = cam.getFrustumable()->getFrustumableTimestamp();
	if(camTimestamp < planes4UpdateTimestamp && prevCam == &cam)
	{
		// Early exit because:
		// - it is the same camera as before and
		// - the camera frustum have not changed
		return;
	}
#if 0
	ANKI_LOGI("Updating 4 planes");
#endif

	// Update the planes in parallel
	// 
	ThreadPool& threadPool = ThreadPoolSingleton::get();

	// Dont even think of defining that in the switch
	Array<UpdateTiles4PlanesPerspectiveCameraJob, ThreadPool::MAX_THREADS> jobs;

	switch(cam.getCameraType())
	{
	case Camera::CT_PERSPECTIVE:
		for(U i = 0; i < threadPool.getThreadsCount(); i++)
		{	
			jobs[i].tiler = this;
			jobs[i].cam = static_cast<PerspectiveCamera*>(&cam);
			threadPool.assignNewJob(i, &jobs[i]);
		}
		break;
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	threadPool.waitForAllJobsToFinish();

	planes4UpdateTimestamp = Timestamp::getTimestamp();
}

//==============================================================================
void Tiler::update2Planes(Camera& cam, 
	F32 (*pixels)[TILES_Y_COUNT][TILES_X_COUNT][2])
{
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	UpdateTiles2PlanesJob jobs[ThreadPool::MAX_THREADS];
	
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].pixels = pixels;
		jobs[i].tiler = this;
		jobs[i].cam = &cam;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
}

//==============================================================================
Bool Tiler::test(const CollisionShape& cs,
	Array<U32, TILES_X_COUNT * TILES_Y_COUNT>* tileIds) const
{
	if(tileIds)
	{
		U32* ip = &(*tileIds)[0];
		for(U i = 0; i < tiles1d.getSize(); i++)
		{
			Bool inside = true;
			for(const Plane& plane : tiles1d[i].planesWSpace)
			{
				if(cs.testPlane(plane) < 0.0)
				{
					inside = false;
					break;
				}
			}

			if(inside)
			{
				*ip++ = i;
			}
		}

		return ip != &(*tileIds)[0];
	}
	else
	{
		for(const Tile& tile : tiles1d)
		{
			Bool inside = true;
			for(const Plane& plane : tile.planesWSpace)
			{
				if(cs.testPlane(plane) < 0.0)
				{
					inside = false;
					break;
				}
			}

			if(inside)
			{
				return true;
			}
		}

		return false;
	}
}

} // end namespace anki
