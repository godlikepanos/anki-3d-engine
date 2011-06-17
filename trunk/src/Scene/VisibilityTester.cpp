#include <boost/foreach.hpp>
#include "VisibilityTester.h"
#include "Scene.h"
#include "ModelNode.h"
#include "SkinNode.h"
#include "ModelPatchNode.h"
#include "Material.h"
#include "Sphere.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "JobManager.h"


//======================================================================================================================
// CmpDistanceFromOrigin::operator()                                                                                   =
//======================================================================================================================
bool VisibilityTester::CmpDistanceFromOrigin::operator()(const SceneNode* a, const SceneNode* b) const
{
	return (a->getWorldTransform().getOrigin() - o).getLengthSquared() <
	       (b->getWorldTransform().getOrigin() - o).getLengthSquared();
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
VisibilityTester::VisibilityTester(Scene& scene_):
	scene(scene_)
{}


//======================================================================================================================
// test                                                                                                                =
//======================================================================================================================
void VisibilityTester::test(Camera& cam)
{
	//
	// Set all nodes to not visible
	//
	BOOST_FOREACH(SceneNode* node, scene.getAllNodes())
	{
		node->setVisible(false);
	}

	//
	// Collect the lights for the main cam
	//
	cam.getVisiblePointLights().clear();
	cam.getVisibleSpotLights().clear();

	BOOST_FOREACH(Light* light, scene.getLights())
	{
		switch(light->getType())
		{
			// Point
			case Light::LT_POINT:
			{
				PointLight* pointl = static_cast<PointLight*>(light);

				Sphere sphere(pointl->getWorldTransform().getOrigin(), pointl->getRadius());
				if(cam.insideFrustum(sphere))
				{
					cam.getVisiblePointLights().push_back(pointl);
					pointl->setVisible(true);
				}
				break;
			}
			// Spot
			case Light::LT_SPOT:
			{
				SpotLight* spotl = static_cast<SpotLight*>(light);

				if(cam.insideFrustum(spotl->getCamera()))
				{
					cam.getVisibleSpotLights().push_back(spotl);
					spotl->setVisible(true);
					spotl->getCamera().setVisible(true);
				}
				break;
			}
		} // end switch
	} // end for all lights

	//
	// Get the renderables for the main cam
	//
	getRenderableNodes(false, cam, cam);

	//
	// For every spot light camera collect the renderable nodes
	//
	BOOST_FOREACH(SpotLight* spot, cam.getVisibleSpotLights())
	{
		getRenderableNodes(true, spot->getCamera(), *spot);
	}
}


//======================================================================================================================
// test                                                                                                                =
//======================================================================================================================
template<typename Type>
bool VisibilityTester::test(const Type& tested, const Camera& cam)
{
	return cam.insideFrustum(tested.getVisibilityShapeWSpace());
}


//======================================================================================================================
// getRenderableNodes                                                                                                  =
//======================================================================================================================
void VisibilityTester::getRenderableNodes(bool skipShadowless_, const Camera& cam_, VisibilityInfo& storage)
{
	cam = &cam_;
	skipShadowless = skipShadowless_;
	visibilityInfo = &storage;

	storage.getVisibleMsRenderableNodes().clear();
	storage.getVisibleBsRenderableNodes().clear();

	for(uint i = 0; i < JobManagerSingleton::getInstance().getThreadsNum(); i++)
	{
		JobManagerSingleton::getInstance().assignNewJob(i, getRenderableNodesJobCallback, this);
	}
	JobManagerSingleton::getInstance().waitForAllJobsToFinish();

	/*
	//
	// ModelNodes
	//
	BOOST_FOREACH(ModelNode* node, scene.getModelNodes())
	{
		// Skip if the ModeNode is not visible
		if(!test(*node, *cam))
		{
			continue;
		}

		node->setVisible(true);

		// If visible test every patch individually
		BOOST_FOREACH(ModelPatchNode* modelPatchNode, node->getModelPatchNodes())
		{
			// Skip shadowless
			if(skipShadowless && !modelPatchNode->getCpMtl().castsShadow())
			{
				continue;
			}

			// Test if visible by main camera
			if(test(*modelPatchNode, *cam))
			{
				if(modelPatchNode->getCpMtl().renderInBlendingStage())
				{
					storage.getVisibleBsRenderableNodes().push_back(modelPatchNode);
				}
				else
				{
					storage.getVisibleMsRenderableNodes().push_back(modelPatchNode);
				}
				modelPatchNode->setVisible(true);
			}
		}
	}

	//
	// SkinNodes
	//
	BOOST_FOREACH(SkinNode* node, scene.getSkinNodes())
	{
		// Skip if the SkinNode is not visible
		if(!test(*node, *cam))
		{
			continue;
		}

		node->setVisible(true);

		// Put all the patches into the visible container
		BOOST_FOREACH(SkinPatchNode* patchNode, node->getPatcheNodes())
		{
			// Skip shadowless
			if(skipShadowless && !patchNode->getCpMtl().castsShadow())
			{
				continue;
			}

			if(patchNode->getCpMtl().renderInBlendingStage())
			{
				storage.getVisibleBsRenderableNodes().push_back(patchNode);
			}
			else
			{
				storage.getVisibleMsRenderableNodes().push_back(patchNode);
			}
			patchNode->setVisible(true);
		}
	}*/

	//
	// Sort the renderables from closest to the camera to the farthest
	//
	std::sort(storage.getVisibleMsRenderableNodes().begin(), storage.getVisibleMsRenderableNodes().end(),
	          CmpDistanceFromOrigin(cam->getWorldTransform().getOrigin()));
	std::sort(storage.getVisibleBsRenderableNodes().begin(), storage.getVisibleBsRenderableNodes().end(),
	          CmpDistanceFromOrigin(cam->getWorldTransform().getOrigin()));
}


//======================================================================================================================
// getRenderableNodesJobCallback                                                                                       =
//======================================================================================================================
void VisibilityTester::getRenderableNodesJobCallback(void* args, const WorkerThread& workerThread)
{
	uint id = workerThread.getId();
	uint threadsNum = workerThread.getJobManager().getThreadsNum();
	VisibilityTester* visTester = reinterpret_cast<VisibilityTester*>(args);
	Scene& scene = visTester->scene;

	uint count, from, to;
	size_t nodesSize;

	//
	// ModelNodes
	//
	nodesSize = scene.getModelNodes().size();
	count = nodesSize / threadsNum;
	from = count * id;
	to = count * (id + 1);

	if(id == threadsNum - 1) // The last job will get the rest
	{
		to = nodesSize;
	}


	Scene::Types<ModelNode>::Iterator it = scene.getModelNodes().begin() + from;
	while(it != scene.getModelNodes().begin() + to)
	{
		ASSERT(it != scene.getModelNodes().end());
		ModelNode* node = *it;

		// Skip if the ModeNode is not visible
		if(!test(*node, *visTester->cam))
		{
			continue;
		}

		node->setVisible(true);

		// If visible test every patch individually
		BOOST_FOREACH(ModelPatchNode* modelPatchNode, node->getModelPatchNodes())
		{
			// Skip shadowless
			if(visTester->skipShadowless && !modelPatchNode->getCpMtl().castsShadow())
			{
				continue;
			}

			// Test if visible by main camera
			if(test(*modelPatchNode, *visTester->cam))
			{
				if(modelPatchNode->getCpMtl().renderInBlendingStage())
				{
					boost::mutex::scoped_lock lock(visTester->bsRenderableNodesMtx);
					visTester->visibilityInfo->getVisibleBsRenderableNodes().push_back(modelPatchNode);
				}
				else
				{
					boost::mutex::scoped_lock lock(visTester->msRenderableNodesMtx);
					visTester->visibilityInfo->getVisibleMsRenderableNodes().push_back(modelPatchNode);
				}
				modelPatchNode->setVisible(true);
			}
		}
	}


	//
	// SkinNodes
	//
	{
		nodesSize = scene.getSkinNodes().size();
		count = nodesSize / threadsNum;
		from = count * id;
		to = count * (id + 1);

		if(id == threadsNum - 1) // The last job will get the rest
		{
			to = nodesSize;
		}

		Scene::Types<SkinNode>::Iterator it = scene.getSkinNodes().begin() + from;
		while(it != scene.getSkinNodes().begin() + to)
		{
			ASSERT(it != scene.getSkinNodes().end());
			SkinNode* node = *it;

			// Skip if the SkinNode is not visible
			if(!test(*node, *visTester->cam))
			{
				continue;
			}

			node->setVisible(true);

			// Put all the patches into the visible container
			BOOST_FOREACH(SkinPatchNode* patchNode, node->getPatcheNodes())
			{
				// Skip shadowless
				if(visTester->skipShadowless && !patchNode->getCpMtl().castsShadow())
				{
					continue;
				}

				if(patchNode->getCpMtl().renderInBlendingStage())
				{
					boost::mutex::scoped_lock lock(visTester->bsRenderableNodesMtx);
					visTester->visibilityInfo->getVisibleBsRenderableNodes().push_back(patchNode);
				}
				else
				{
					boost::mutex::scoped_lock lock(visTester->msRenderableNodesMtx);
					visTester->visibilityInfo->getVisibleMsRenderableNodes().push_back(patchNode);
				}
				patchNode->setVisible(true);
			}
		}
	}
}
