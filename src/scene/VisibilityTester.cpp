#include <boost/foreach.hpp>
#include "VisibilityTester.h"
#include "Scene.h"
#include "ModelNode.h"
#include "SkinNode.h"
#include "ModelPatchNode.h"
#include "resource/Material.h"
#include "collision/Sphere.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "core/parallel/Manager.h"
#include "core/Logger.h"


//==============================================================================
// Destructor                                                                  =
//==============================================================================
VisibilityTester::~VisibilityTester()
{}


//==============================================================================
// CmpDistanceFromOrigin::operator()                                           =
//==============================================================================
bool VisibilityTester::CmpDistanceFromOrigin::operator()(const SceneNode* a,
	const SceneNode* b) const
{
	return (a->getWorldTransform().getOrigin() - o).getLengthSquared() <
		(b->getWorldTransform().getOrigin() - o).getLengthSquared();
}


//==============================================================================
// Constructor                                                                 =
//==============================================================================
VisibilityTester::VisibilityTester(Scene& scene_):
	scene(scene_)
{}


//==============================================================================
// test                                                                        =
//==============================================================================
void VisibilityTester::test(Camera& cam)
{
	//
	// Set all nodes to not visible
	//
	BOOST_FOREACH(SceneNode* node, scene.getAllNodes())
	{
		node->disableFlag(SceneNode::SNF_VISIBLE);
	}

	//
	// Collect the lights for the main cam
	//
	cam.getVisiblePointLights().clear();
	cam.getVisibleSpotLights().clear();

	BOOST_FOREACH(Light* light, scene.getLights())
	{
		if(!light->isFlagEnabled(SceneNode::SNF_ACTIVE))
		{
			continue;
		}

		switch(light->getLightType())
		{
			// Point
			case Light::LT_POINT:
			{
				PointLight* pointl = static_cast<PointLight*>(light);

				Sphere sphere(pointl->getWorldTransform().getOrigin(),
					pointl->getRadius());
				if(cam.insideFrustum(sphere))
				{
					cam.getVisiblePointLights().push_back(pointl);
					pointl->enableFlag(SceneNode::SNF_VISIBLE);
				}
				break;
			}
			// Spot
			case Light::LT_SPOT:
			{
				SpotLight* spotl = static_cast<SpotLight*>(light);

				if(cam.insideFrustum(*(spotl->getCamera().
					getVisibilityCollisionShapeWorldSpace())))
				{
					cam.getVisibleSpotLights().push_back(spotl);
					spotl->enableFlag(SceneNode::SNF_VISIBLE);
					spotl->getCamera().enableFlag(SceneNode::SNF_VISIBLE);
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


//==============================================================================
// test                                                                        =
//==============================================================================
template<typename Type>
bool VisibilityTester::test(const Type& tested, const Camera& cam)
{
	return cam.insideFrustum(*tested.getVisibilityCollisionShapeWorldSpace());
}


//==============================================================================
// getRenderableNodes                                                          =
//==============================================================================
void VisibilityTester::getRenderableNodes(bool skipShadowless,
	const Camera& cam, VisibilityInfo& storage)
{
	storage.getVisibleMsRenderableNodes().clear();
	storage.getVisibleBsRenderableNodes().clear();

	// Run in parallel
	VisJobParameters jobParameters;
	jobParameters.cam = &cam;
	jobParameters.skipShadowless = skipShadowless;
	jobParameters.visibilityInfo = &storage;
	jobParameters.scene = &scene;
	jobParameters.msRenderableNodesMtx = &msRenderableNodesMtx;
	jobParameters.bsRenderableNodesMtx = &bsRenderableNodesMtx;

	for(uint i = 0;
		i < parallel::ManagerSingleton::get().getThreadsNum(); i++)
	{
		parallel::ManagerSingleton::get().assignNewJob(i,
			getRenderableNodesJobCallback, jobParameters);
	}
	parallel::ManagerSingleton::get().waitForAllJobsToFinish();

	//
	// Sort the renderables from closest to the camera to the farthest
	//
	std::sort(storage.getVisibleMsRenderableNodes().begin(),
		storage.getVisibleMsRenderableNodes().end(),
		CmpDistanceFromOrigin(cam.getWorldTransform().getOrigin()));
	std::sort(storage.getVisibleBsRenderableNodes().begin(),
		storage.getVisibleBsRenderableNodes().end(),
		CmpDistanceFromOrigin(cam.getWorldTransform().getOrigin()));
}


//==============================================================================
// getRenderableNodesJobCallback                                               =
//==============================================================================
void VisibilityTester::getRenderableNodesJobCallback(
	parallel::JobParameters& data,
	const parallel::Job& job)
{
	VisJobParameters& jobParameters = static_cast<VisJobParameters&>(data);

	uint id = job.getId();
	uint threadsNum = job.getManager().getThreadsNum();

	const Camera& cam = *jobParameters.cam;
	bool skipShadowless = jobParameters.skipShadowless;
	VisibilityInfo& visibilityInfo = *jobParameters.visibilityInfo;
	Scene& scene = *jobParameters.scene;
	boost::mutex& msRenderableNodesMtx = *jobParameters.msRenderableNodesMtx;
	boost::mutex& bsRenderableNodesMtx = *jobParameters.bsRenderableNodesMtx;

	uint count, from, to;
	size_t nodesSize;

	boost::array<const RenderableNode*, Scene::MAX_VISIBLE_NODES> msVisibles,
		bsVisibles;
	uint msI = 0, bsI = 0;


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

	for(uint i = from; i < to; i++)
	{
		ModelNode* node = scene.getModelNodes()[i];

		if(!node->isFlagEnabled(SceneNode::SNF_ACTIVE))
		{
			continue;
		}

		// Skip if the ModeNode is not visible
		if(!test(*node, cam))
		{
			continue;
		}

		node->enableFlag(SceneNode::SNF_VISIBLE);

		// If visible test every patch individually
		BOOST_FOREACH(ModelPatchNode* modelPatchNode,
			node->getModelPatchNodes())
		{
			if(!modelPatchNode->isFlagEnabled(SceneNode::SNF_ACTIVE))
			{
				continue;
			}

			// Skip shadowless
			if(skipShadowless &&
				!modelPatchNode->getMaterialRuntime().getCastShadow())
			{
				continue;
			}

			// Test if visible by main camera
			if(test(*modelPatchNode, cam))
			{
				if(modelPatchNode->getMaterialRuntime().
					getRenderInBledingStage())
				{
					bsVisibles[bsI++] = modelPatchNode;
				}
				else
				{
					msVisibles[msI++] = modelPatchNode;
				}
				modelPatchNode->enableFlag(SceneNode::SNF_VISIBLE);
			}
		}
	}


	//
	// SkinNodes
	//
	nodesSize = scene.getSkinNodes().size();
	count = nodesSize / threadsNum;
	from = count * id;
	to = count * (id + 1);

	if(id == threadsNum - 1) // The last job will get the rest
	{
		to = nodesSize;
	}

	for(uint i = from; i < to; i++)
	{
		SkinNode* node = scene.getSkinNodes()[i];

		if(!node->isFlagEnabled(SceneNode::SNF_ACTIVE))
		{
			continue;
		}

		// Skip if the SkinNode is not visible
		if(!test(*node, cam))
		{
			continue;
		}

		node->enableFlag(SceneNode::SNF_VISIBLE);

		// Put all the patches into the visible container
		BOOST_FOREACH(SkinPatchNode* patchNode,
			node->getPatchNodes())
		{
			if(!patchNode->isFlagEnabled(SceneNode::SNF_ACTIVE))
			{
				continue;
			}

			// Skip shadowless
			if(skipShadowless &&
				!patchNode->getMaterialRuntime().getCastShadow())
			{
				continue;
			}

			if(patchNode->getMaterialRuntime().getRenderInBledingStage())
			{
				bsVisibles[bsI++] = patchNode;
			}
			else
			{
				msVisibles[msI++] = patchNode;
			}
			patchNode->enableFlag(SceneNode::SNF_VISIBLE);
		}
	}


	//
	// Copy the temps to the global
	//
	{
		boost::mutex::scoped_lock lock(bsRenderableNodesMtx);

		for(uint i = 0; i < bsI; i++)
		{
			visibilityInfo.getVisibleBsRenderableNodes().push_back(
				bsVisibles[i]);
		}
	}

	{
		boost::mutex::scoped_lock lock(msRenderableNodesMtx);

		for(uint i = 0; i < msI; i++)
		{
			visibilityInfo.getVisibleMsRenderableNodes().push_back(
				msVisibles[i]);
		}
	}
}
