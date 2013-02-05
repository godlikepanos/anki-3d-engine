#include "anki/scene/VisibilityTester.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/ThreadPool.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct DistanceSortFunctor
{
	Vec3 origin;

	Bool operator()(SceneNode* a, SceneNode* b)
	{
		ANKI_ASSERT(a->getSpatial() != nullptr && b->getSpatial() != nullptr);

		F32 dist0 = origin.getDistanceSquared(
			a->getSpatial()->getSpatialOrigin());
		F32 dist1 = origin.getDistanceSquared(
			b->getSpatial()->getSpatialOrigin());

		return dist0 < dist1;
	}
};

//==============================================================================
struct MaterialSortFunctor
{
	Bool operator()(SceneNode* a, SceneNode* b)
	{
		ANKI_ASSERT(a->getRenderable() != nullptr 
			&& b->getRenderable() != nullptr);

		return a->getRenderable()->getRenderableMaterial()
			< b->getRenderable()->getRenderableMaterial();
	}
};

//==============================================================================
struct VisibilityTestJob: ThreadJob
{
	U nodesCount;
	Scene::Types<SceneNode>::Container::iterator nodes;
	std::mutex* renderablesMtx;
	std::mutex* lightsMtx;
	Frustumable* frustumable;
	Renderer* renderer;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, nodesCount, start, end);
		const U TEMP_STORE_COUNT = 512;
		Array<SceneNode*, TEMP_STORE_COUNT> tmpRenderables;
		Array<SceneNode*, TEMP_STORE_COUNT> tmpLights;
		U renderablesIdx = 0;
		U lightsIdx = 0;

		for(auto it = nodes + start; it != nodes + end; it++)
		{
			SceneNode* node = *it;

			Frustumable* fr = node->getFrustumable();
			// Skip if it is the same
			if(frustumable == fr)
			{
				continue;
			}

			Spatial* sp = node->getSpatial();
			if(!sp)
			{
				continue;
			}

			if(!frustumable->insideFrustum(*sp))
			{
				continue;
			}

			Renderable* r = node->getRenderable();
			if(r)
			{
				if(!renderer->doVisibilityTests(
					sp->getOptimalCollisionShape()))
				{
					continue;
				}
				tmpRenderables[renderablesIdx++] = node;
			}
			else
			{
				Light* l = node->getLight();
				if(l)
				{
					tmpLights[lightsIdx++] = node;

					if(l->getShadowEnabled() && fr)
					{
						testLight(*l);
					}
				}
			}

			sp->enableFlags(Spatial::SF_VISIBLE_CAMERA);
		} // end for

		VisibilityInfo& vinfo = frustumable->getVisibilityInfo();

		// Write to containers
		if(renderablesIdx > 0)
		{
			std::lock_guard<std::mutex> lock(*renderablesMtx);

			vinfo.renderables.insert(vinfo.renderables.begin(),
				&tmpRenderables[0],
				&tmpRenderables[renderablesIdx]);
		}

		if(lightsIdx > 0)
		{
			std::lock_guard<std::mutex> lock(*lightsMtx);

			vinfo.lights.insert(vinfo.lights.begin(),
				&tmpLights[0],
				&tmpLights[lightsIdx]);
		}
	}

	void testLight(Light& light)
	{
		Frustumable& ref = *light.getFrustumable();
		ANKI_ASSERT(&ref != nullptr);

		VisibilityInfo& vinfo = ref.getVisibilityInfo();
		vinfo.renderables.clear();
		vinfo.lights.clear();

		for(auto it = nodes; it != nodes + nodesCount; it++)
		{
			SceneNode* node = *it;

			Frustumable* fr = node->getFrustumable();
			// Wont check the same
			if(&ref == fr)
			{
				continue;
			}

			Spatial* sp = node->getSpatial();
			if(!sp)
			{
				continue;
			}

			if(!ref.insideFrustum(*sp))
			{
				continue;
			}

			sp->enableFlags(Spatial::SF_VISIBLE_LIGHT);

			Renderable* r = node->getRenderable();
			if(r)
			{
				vinfo.renderables.push_back(node);
			}
		}
	}
};

//==============================================================================
struct DistanceSortJob: ThreadJob
{
	U nodesCount;
	VisibilityInfo::Renderables::iterator nodes;
	Vec3 origin;

	void operator()(U threadId, U threadsCount)
	{
		DistanceSortFunctor comp;
		comp.origin = origin;
		std::sort(nodes, nodes + nodesCount, comp);
	}
};

//==============================================================================
struct MaterialSortJob: ThreadJob
{
	U nodesCount;
	VisibilityInfo::Renderables::iterator nodes;

	void operator()(U threadId, U threadsCount)
	{
		std::sort(nodes, nodes + nodesCount, MaterialSortFunctor());
	}
};

//==============================================================================
VisibilityTester::~VisibilityTester()
{}

//==============================================================================
void VisibilityTester::test(Frustumable& ref, Scene& scene, Renderer& r)
{
	// Set all spatials to not visible
	for(auto it = scene.getSceneNodesBegin(); it != scene.getSceneNodesEnd(); ++it)
	{
		Spatial* sp = (*it)->getSpatial();

		if(sp)
		{
			sp->disableFlags(Spatial::SF_VISIBLE_ANY);
		}
	}

	VisibilityInfo& vinfo = ref.getVisibilityInfo();
	vinfo.renderables.clear();
	vinfo.lights.clear();

	ThreadPool& threadPool = ThreadPoolSingleton::get();
	VisibilityTestJob jobs[ThreadPool::MAX_THREADS];

	std::mutex renderablesMtx;
	std::mutex lightsMtx;

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].nodesCount = scene.getSceneNodesCount();
		jobs[i].nodes = scene.getSceneNodesBegin();
		jobs[i].renderablesMtx = &renderablesMtx;
		jobs[i].lightsMtx = &lightsMtx;
		jobs[i].frustumable = &ref;
		jobs[i].renderer = &r;

		threadPool.assignNewJob(i, &jobs[i]);
	}

	threadPool.waitForAllJobsToFinish();

	// Sort
	//

	// The lights
	DistanceSortJob dsjob;
	dsjob.nodes = vinfo.lights.begin();
	dsjob.nodesCount = vinfo.lights.size();
	dsjob.origin = ref.getFrustumableOrigin();
	threadPool.assignNewJob(0, &dsjob);

	// The rest of the jobs are dummy
	ThreadJobDummy dummyjobs[ThreadPool::MAX_THREADS];
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewJob(i, &dummyjobs[i]);
	}

	// Sort the renderables in the main thread
	std::sort(vinfo.renderables.begin(), 
		vinfo.renderables.end(), MaterialSortFunctor());

	threadPool.waitForAllJobsToFinish();
}

} // end namespace anki
