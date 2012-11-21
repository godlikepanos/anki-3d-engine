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

	bool operator()(SceneNode* a, SceneNode* b)
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
	// XXX
};

//==============================================================================
struct TestJob: ThreadJob
{
	U count;
	Scene::Types<SceneNode>::Container::iterator nodes;
	VisibilityInfo::Renderables* renderables;
	VisibilityInfo::Lights* lights;
	std::mutex* renderablesMtx;
	std::mutex* lightsMtx;
	Frustumable* frustumable;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, count, start, end);
		const U TEMP_STORE_COUNT = 128;
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

			sp->disableFlag(Spatial::SF_VISIBLE);

			if(!frustumable->insideFrustum(*sp))
			{
				continue;
			}

			/*if(!r.doVisibilityTests(sp->getAabb()))
			{
				continue;
			}*/

			sp->enableFlag(Spatial::SF_VISIBLE);

			Renderable* r = node->getRenderable();
			if(r)
			{
				tmpRenderables[renderablesIdx++] = node;
			}
			else
			{
				Light* l = node->getLight();
				if(l)
				{
					tmpLights[lightsIdx++] = node;

					/*if(l->getShadowEnabled() && fr)
					{
						testLight(*l, scene);
					}*/
				}
			}
		} // end for

		// Write to containers
		if(renderablesIdx > 0)
		{
			std::lock_guard<std::mutex> lock(*renderablesMtx);

			renderables->insert(renderables->begin(),
				&tmpRenderables[0],
				&tmpRenderables[renderablesIdx]);
		}

		if(lightsIdx > 0)
		{
			std::lock_guard<std::mutex> lock(*lightsMtx);

			lights->insert(lights->begin(),
				&tmpLights[0],
				&tmpLights[lightsIdx]);
		}
	}
};

//==============================================================================
VisibilityTester::~VisibilityTester()
{}

//==============================================================================
void VisibilityTester::test(Frustumable& ref, Scene& scene, Renderer& r)
{
	VisibilityInfo& vinfo = ref.getVisibilityInfo();
	vinfo.renderables.clear();
	vinfo.lights.clear();

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
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

		sp->disableFlag(Spatial::SF_VISIBLE);

		if(!ref.insideFrustum(*sp))
		{
			continue;
		}

		/*if(!r.doVisibilityTests(sp->getAabb()))
		{
			continue;
		}*/

		sp->enableFlag(Spatial::SF_VISIBLE);

		Renderable* r = node->getRenderable();
		if(r)
		{
			vinfo.renderables.push_back(node);
		}
		else
		{
			Light* l = node->getLight();
			if(l)
			{
				vinfo.lights.push_back(node);

				if(l->getShadowEnabled() && fr)
				{
					testLight(*l, scene);
				}
			}
		}
	}

	DistanceSortFunctor comp;
	comp.origin =
		ref.getSceneNode().getMovable()->getWorldTransform().getOrigin();
	std::sort(vinfo.lights.begin(), vinfo.lights.end(), comp);

	std::sort(vinfo.renderables.begin(), vinfo.renderables.end(), comp);
}

//==============================================================================
void VisibilityTester::testLight(Light& light, Scene& scene)
{
	Frustumable& ref = *light.getFrustumable();
	ANKI_ASSERT(&ref != nullptr);

	VisibilityInfo& vinfo = ref.getVisibilityInfo();
	vinfo.renderables.clear();
	vinfo.lights.clear();

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
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

		sp->enableFlag(Spatial::SF_VISIBLE);

		Renderable* r = node->getRenderable();
		if(r)
		{
			vinfo.renderables.push_back(node);
		}
	}
}

} // end namespace anki
