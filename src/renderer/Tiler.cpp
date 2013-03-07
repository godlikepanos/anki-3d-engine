#include "anki/renderer/Tiler.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/core/ThreadPool.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Spatial.h"

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
// Statics                                                                     =
//==============================================================================

//==============================================================================
struct ShortLexicographicallyFunctor
{
	Bool operator()(const Vec2& a, const Vec2& b)
	{
		return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
	}
};

//==============================================================================
static Bool isLeft(const Vec2& p0, const Vec2& p1, const Vec2& p2)
{
	return (p1.x() - p0.x()) * (p2.y() - p0.y())
		> (p2.x() - p0.x()) * (p1.y() - p0.y());
}

//==============================================================================
static void convexHull2D(Vec2* ANKI_RESTRICT inPoints,
	const U32 n, Vec2* ANKI_RESTRICT outPoints, const U32 on, U32& outn)
{
	ANKI_ASSERT(on > (2 * n) && "This algorithm needs some space");
	I k = 0;

	std::sort(&inPoints[0], &inPoints[n], ShortLexicographicallyFunctor());

	// Build lower hull
	for(I i = 0; i < n; i++)
	{
		while(k >= 2 
			&& !isLeft(outPoints[k - 2], outPoints[k - 1], inPoints[i]))
		{
			--k;
		}

		outPoints[k++] = inPoints[i];
	}

	// Build upper hull
	for(I i = n - 2, t = k + 1; i >= 0; i--)
	{
		while(k >= t 
			&& !isLeft(outPoints[k - 2], outPoints[k - 1], inPoints[i]))
		{
			--k;
		}

		outPoints[k++] = inPoints[i];
	}

	outn = k;
}

//==============================================================================
static U getTilesCount(U maxDepth)
{
	if(maxDepth == 0)
	{
		return 1;
	}

	return getTilesCount(maxDepth - 1) + pow(4, maxDepth);
}

//==============================================================================
static U32 setNBits(U32 n)
{
	U32 res = 0;

	for(U32 i = 0; i < n; i++)
	{
		res |= 0x80000000 >> i;
	}

	return res;
}

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

	// Tiles
	initTiles();
}

//==============================================================================
void Tiler::initTiles()
{
	// Init tiles
	U maxDepth = log2(TILES_X_COUNT);
	tiles_.resize(getTilesCount(maxDepth));

	Tile_* tmpTiles = &tiles_[0];
	for(U d = 0; d < maxDepth + 1; d++)
	{
		tiles0 = tmpTiles;
		tmpTiles = initTilesInDepth(tmpTiles, d);
	}

	// Init hierarchy
	U offset = 0;
	tmpTiles = &tiles_[0];
	for(U d = 0; d < maxDepth; d++)
	{
		U axisCount = pow(2, d);

		U count = axisCount * axisCount;
		offset += count;

		for(U j = 0; j < axisCount; j++)
		{
			for(U i = 0; i < axisCount; i++)
			{
				U k = j * axisCount + i;
				Tile_& tile = tmpTiles[k];

				for(U j_ = 0; j_ < 2; j_++)
				{
					for(U i_ = 0; i_ < 2; i_++)
					{
						tile.children[j_ * 2 + i_] =
							(2 * j + j_) * axisCount * 2 + (2 * i + i_)
							+ offset;
					}
				}
			}
		}

		tmpTiles = tmpTiles + count;
	}
}

//==============================================================================
Tiler::Tile_* Tiler::initTilesInDepth(Tile_* tiles, U depth)
{
	U crntDepthAxisCount = pow(2, depth);

	F32 dim = (1.0 - (-1.0)) / crntDepthAxisCount;

	const U bits = TILES_X_COUNT / crntDepthAxisCount;
	const U32 maskOfTile = setNBits(bits);

	for(U j = 0; j < crntDepthAxisCount; j++)
	{
		for(U i = 0; i < crntDepthAxisCount; i++)
		{
			Tile_& tile = tiles[j * crntDepthAxisCount + i];

			tile.min[0] = -1.0 + i * dim;
			tile.min[1] = -1.0 + j * dim;
			tile.max[0] = tile.min[0] + dim;
			tile.max[1] = tile.min[1] + dim;

			tile.mask[0] = (maskOfTile >> (i * bits));
			tile.mask[1] = (maskOfTile >> (j * bits));

			tile.children[0] = tile.children[1] = tile.children[2] =
				tile.children[3] = -1;
		}
	}

	return tiles + (crntDepthAxisCount * crntDepthAxisCount);
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
void Tiler::updateTilesInternal()
{
	//
	// Read the results from the minmax job
	//
	Array<F32, TILES_X_COUNT * TILES_X_COUNT * 2> pixels;
	pbo.read(&pixels[0]);

	//
	// Set the Z of the level 0 tiles
	//
	for(U i = 0; i < TILES_X_COUNT * TILES_X_COUNT; i++)
	{
		tiles0[i].min.z() = pixels[i * 2 + 0];
		tiles0[i].max.z() = pixels[i * 2 + 1];
	}

	//
	// Move to the other levels
	//
	updateTileMinMax(tiles_[0]);
}

//==============================================================================
Vec2 Tiler::updateTileMinMax(Tile_& tile)
{
	// Have children?
	if(tile.children[0] == -1)
	{
		return Vec2(tile.min.z(), tile.max.z());
	}

	Vec2 minMax(10.0, -10.0);
	for(U i = 0; i < 4; i++)
	{
		Vec2 in = updateTileMinMax(tiles_[tile.children[i]]);
		minMax.x() = std::min(minMax.x(), in.x());
		minMax.y() = std::max(minMax.y(), in.y());
	}

	tile.min.z() = minMax.x();
	tile.max.z() = minMax.y();

	return minMax;
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

//==============================================================================
Bool Tiler::testAll(const CollisionShape& cs,
 	U32* tileIds, U32& tilesCount, const Bool skipNearPlaneCheck) const
{
	U startPlane = (skipNearPlaneCheck) ? 1 : 0;
	tilesCount = 0;

	for(U i = 0; i < tiles1d.getSize(); i++)
	{
		const Tile& tile = tiles1d[i];

		if(testInternal(cs, tile, startPlane))
		{
			tileIds[tilesCount++] = i;
		}
	}

	return tilesCount > 0;
}

//==============================================================================
Bool Tiler::test(const Spatial& sp, const Frustumable& fr) const
{
	//
	// Get points from sp
	//
	Array<Vec4, 8> points;
	U pointsCount = 0;

	const CollisionShape& cs = sp.getSpatialCollisionShape();
	if(cs.getCollisionShapeType() != CollisionShape::CST_FRUSTUM)
	{
		const Aabb& aabb = sp.getAabb();
		const Vec3& min = aabb.getMin();
		const Vec3& max = aabb.getMax();

		points[0] = Vec4(max.x(), max.y(), max.z(), 1.0); // right top front
		points[1] = Vec4(min.x(), max.y(), max.z(), 1.0); // left top front
		points[2] = Vec4(min.x(), min.y(), max.z(), 1.0); // left bottom front
		points[3] = Vec4(max.x(), min.y(), max.z(), 1.0); // right bottom front
		points[4] = Vec4(max.x(), max.y(), min.z(), 1.0); // right top back
		points[5] = Vec4(min.x(), max.y(), min.z(), 1.0); // left top back
		points[6] = Vec4(min.x(), min.y(), min.z(), 1.0); // left bottom back
		points[7] = Vec4(max.x(), min.y(), min.z(), 1.0); // right bottom back

		pointsCount = 0;
	}
	else
	{
		// XXX
	}

	//
	// Transform those shapes
	//
	const Mat4& mat = fr.getViewProjectionMatrix();
	Array<Vec2, 8> points2D;

	Vec2 minMax(10.0, -10.0); // The min and max z of the transformed shape
	for(U i = 0; i < pointsCount; i++)
	{
		Vec4 point = mat * points[i];
		Vec3 v3 = point.xyz() / point.w();
		points2D[i] = v3.xy();

		minMax.x() = std::min(minMax.x(), v3.z());
		minMax.y() = std::max(minMax.y(), v3.z());
	}

	//
	// Calc the convex hull
	//
	Array<Vec2, 8 * 2> convPoints;
	U32 convPointsCount;
	convexHull2D(&points2D[0], pointsCount, 
			&convPoints[0], convPoints.getSize(), convPointsCount);

	//
	// Run the algorithm for every edge
	//
	Array<U32, 2> mask = {{0, 0}};
	for(U i = 0; i < convPointsCount - 1; i++)
	{
		testTile(tiles_[0], minMax, convPoints[i], convPoints[i + 1], mask);
	}

	return mask[0] != 0 && mask[1] != 0;
}

//==============================================================================
void Tiler::testTile(const Tile_& tile, const Vec2& a, const Vec2& b, 
	const Vec2& objectMinMaxZ, Array<U32, 2>& mask) const
{
	// If edge not inside return
	/*for(U i = 0; i < 2; i++)
	{
		if(a[i] < tile.min[i] || a[i] > tile.max[i])
		{
			return;
		}

		if(b[i] < tile.min[i] || b[i] > tile.max[i])
		{
			return;
		}
	}*/

	Bool final = tile.children[0] == -1;

	U inside = 0;

	if(isLeft(a, b, Vec2(tile.max.x(), tile.max.y())))
	{
		if(final)
		{
			goto allIn;
		}
		++inside;
	}

	if(isLeft(a, b, Vec2(tile.min.x(), tile.max.y())))
	{
		if(final)
		{
			goto allIn;
		}
		++inside;
	}

	if(!isLeft(a, b, Vec2(tile.min.x(), tile.min.y())))
	{
		if(final)
		{
			goto allIn;
		}
		++inside;
	}

	if(!isLeft(a, b, Vec2(tile.max.x(), tile.min.y())))
	{
		if(final)
		{
			goto allIn;
		}
		++inside;
	}

	// None inside
	if(inside == 0)
	{
		return;
	}
	else if(inside == 4) // All inside
	{
		goto allIn;
	}
	else // Some inside
	{
		ANKI_ASSERT(!final);

		for(U i = 0; i < 4; i++)
		{
			testTile(tiles_[tile.children[i]], a, b, objectMinMaxZ, mask);
		}
	}

	return;

allIn:
	mask[0] |= tile.mask[0];
	mask[1] |= tile.mask[1];
}

} // end namespace anki
