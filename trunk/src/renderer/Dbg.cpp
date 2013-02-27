#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Light.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
Dbg::~Dbg()
{}

//==============================================================================
void Dbg::init(const Renderer::Initializer& initializer)
{
	enabled = initializer.dbg.enabled;
	enableFlags(DF_ALL);

	try
	{
		fbo.create();
		fbo.setColorAttachments({&r->getPps().getFai()});
		fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());

		if(!fbo.isComplete())
		{
			throw ANKI_EXCEPTION("FBO is incomplete");
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}

	drawer.reset(new DebugDrawer);
	sceneDrawer.reset(new SceneDebugDrawer(drawer.get()));
}

//==============================================================================
Bool isLeft(const Vec2& p0, const Vec2& p1, const Vec2& p2)
{
	return (p1.x() - p0.x()) * (p2.y() - p0.y())
		- (p2.x() - p0.x()) * (p1.y() - p0.y()) > 0.0;
}

void calcConvexHull2D(const Vec2* points, const U32 n, Vec2* opoints, U32& on)
{
	const U maxPoints = 8;
	ANKI_ASSERT(n <= maxPoints && n > 3);
	Array<Vec2, maxPoints * 2 + 1> deque;

	// initial bottom and top deque indices
	U bot = n - 2, top = bot + 3;
	// 3rd vertex is at both bot and top
	deque[bot] = deque[top] = points[2];

	if(isLeft(points[0], points[1], points[2]))
	{
		deque[bot + 1] = points[0];
		deque[bot + 2] = points[1]; // ccw vertices are: 2, 0, 1, 2
	}
	else
	{
		deque[bot + 1] = points[1];
		deque[bot + 2] = points[0]; // ccw vertices are: 2, 1, 0, 2
	}

	// compute the hull on the deque D[]
	for(U i = 3; i < n; i++)
	{
		// test if next vertex is inside the deque hull
		if ((isLeft(deque[bot], deque[bot + 1], points[i]))
			&& (isLeft(deque[top-1], deque[top], points[i])))
		{
			continue; // skip an interior vertex
		}

		// incrementally add an exterior vertex to the deque hull
		// get the rightmost tangent at the deque bot
		while(!isLeft(deque[bot], deque[bot + 1], points[i]))
		{
			++bot; // remove bot of deque
		}

		deque[--bot] = points[i]; // insert V[i] at bot of deque

		// get the leftmost tangent at the deque top
		while(!isLeft(deque[top-1], deque[top], points[i]))
		{
			--top; // pop top of deque
		}

		deque[++top] = points[i];          // push V[i] onto top of deque
	}

	// transcribe deque [] to the output hull array opoints
	U h;  // hull vertex counter
	for( h = 0; h <= (top - bot); h++)
	{
		opoints[h] = deque[bot + h];
	}

	on = h - 1;
}

void Dbg::run()
{
	ANKI_ASSERT(enabled);

	fbo.bind();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().enable(GL_DEPTH_TEST, depthTest);

	drawer->setViewProjectionMatrix(r->getViewProjectionMatrix());
	drawer->setModelMatrix(Mat4::getIdentity());
	//drawer->drawGrid();

#if 0
	SceneGraph& scene = r->getSceneGraph();

	for(auto it = scene.getSceneNodesBegin();
		it != scene.getSceneNodesEnd(); it++)
	{
		SceneNode* node = *it;
		Spatial* sp = node->getSpatial();
		if(flagsEnabled(DF_SPATIAL) && sp)
		{
			sceneDrawer->draw(*node);
		}
	}

	// Draw sectors
	for(const Sector* sector : scene.getSectorGroup().getSectors())
	{
		//if(sector->isVisible())
		{
			if(flagsEnabled(DF_SECTOR))
			{
				sceneDrawer->draw(*sector);
			}

			if(flagsEnabled(DF_OCTREE))
			{
				sceneDrawer->draw(sector->getOctree());
			}
		}
	}

	// Physics
	if(flagsEnabled(DF_PHYSICS))
	{
		scene.getPhysics().debugDraw();
	}
#endif

	{
		drawer->setColor(Vec3(1.0, 0.0, 1.0));
		Aabb box(Vec3(-1.0, 1.0, -1.0), Vec3(1.5, 2.0, 1.5));
		CollisionDebugDrawer coldrawer(drawer.get());

		box.accept(coldrawer);

		drawer->setColor(Vec3(1.0));

		const Vec3& min = box.getMin();
		const Vec3& max = box.getMax();

		Array<Vec4, 8> points = {{
			Vec4(max.x(), max.y(), max.z(), 1.0),   // right top front
			Vec4(min.x(), max.y(), max.z(), 1.0),   // left top front
			Vec4(min.x(), min.y(), max.z(), 1.0),   // left bottom front
			Vec4(max.x(), min.y(), max.z(), 1.0),   // right bottom front
			Vec4(max.x(), max.y(), min.z(), 1.0),   // right top back
			Vec4(min.x(), max.y(), min.z(), 1.0),   // left top back
			Vec4(min.x(), min.y(), min.z(), 1.0),   // left bottom back
			Vec4(max.x(), min.y(), min.z(), 1.0)}}; // right bottom back

		drawer->setViewProjectionMatrix(Mat4::getIdentity());
		drawer->setModelMatrix(Mat4::getIdentity());

		const Mat4& mat = r->getViewProjectionMatrix();

		Array<Vec2, 8> points2D;
		Array<Vec2, 8> hullPoints2D;

		drawer->begin();
		for(U i = 0; i < 8; i++)
		{
			Vec4& point = points[i];
			point = mat * point;
			point /= point.w();

			points2D[i] = point.xy();

			//drawer->pushBackVertex(Vec3(point.x(), point.y(), -0.1));
		}
		drawer->end();

		U32 outPoints;
		calcConvexHull2D(&points2D[0], 8, &hullPoints2D[0], outPoints);

		drawer->begin();
		for(U i = 0; i < outPoints - 1; i++)
		{
			drawer->pushBackVertex(Vec3(hullPoints2D[i], -0.1));
			drawer->pushBackVertex(Vec3(hullPoints2D[i + 1], -0.1));
		}

		drawer->pushBackVertex(Vec3(hullPoints2D[outPoints - 1], -0.1));
		drawer->pushBackVertex(Vec3(hullPoints2D[0], -0.1));

		drawer->end();
	}

	drawer->flush();
}

} // end namespace anki
