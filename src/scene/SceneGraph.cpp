#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/scene/InstanceNode.h"
#include "anki/util/Exception.h"
#include "anki/core/Threadpool.h"
#include "anki/core/Counters.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/Xml.h"
#include "anki/physics/RigidBody.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct UpdateSceneNodesJob: ThreadpoolTask
{
	SceneGraph* scene = nullptr;
	F32 prevUpdateTime;
	F32 crntTime;
	Barrier* barrier = nullptr;

	void operator()(ThreadId threadId, U threadsCount)
	{
		ANKI_ASSERT(scene);
		U64 start, end;
		choseStartEnd(
			threadId, threadsCount, scene->getSceneNodesCount(), start, end);

		// First update the move components
		scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			MoveComponent* move = node.tryGetComponent<MoveComponent>();

			if(move)
			{
				move->updateReal(node, prevUpdateTime, crntTime,
					SceneComponent::ASYNC_UPDATE);
			}
		});

		barrier->wait();

		// Update the rest of the components
		auto moveComponentTypeId = MoveComponent::getGlobType();
		scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			// Components update
			node.iterateComponents([&](SceneComponent& comp)
			{
				if(comp.getType() != moveComponentTypeId)
				{
					comp.updateReal(node, prevUpdateTime, crntTime, 
						SceneComponent::ASYNC_UPDATE);
				}
			});

			// Frame update
			node.frameUpdate(prevUpdateTime, crntTime, 
				SceneNode::ASYNC_UPDATE);
		});
	}
};

//==============================================================================
// Scene                                                                       =
//==============================================================================

//==============================================================================
SceneGraph::SceneGraph()
	:	alloc(ANKI_SCENE_ALLOCATOR_SIZE),
		frameAlloc(ANKI_SCENE_FRAME_ALLOCATOR_SIZE),
		nodes(alloc),
		dict(10, DictionaryHasher(), DictionaryEqual(), alloc),
		physics(this),
		sectorGroup(this),
		events(this)
{
	nodes.reserve(ANKI_SCENE_OPTIMAL_SCENE_NODES_COUNT);

	ambientCol = Vec3(0.0);
}

//==============================================================================
SceneGraph::~SceneGraph()
{}

//==============================================================================
void SceneGraph::registerNode(SceneNode* node)
{
	ANKI_ASSERT(node);

	// Add to dict if it has name
	if(node->getName())
	{
		if(dict.find(node->getName()) != dict.end())
		{
			throw ANKI_EXCEPTION("Node with the same name already exists: "
				+ node->getName());
		}

		dict[node->getName()] = node;
	}

	// Add to vector
	ANKI_ASSERT(std::find(nodes.begin(), nodes.end(), node) == nodes.end());
	nodes.push_back(node);
}

//==============================================================================
void SceneGraph::unregisterNode(SceneNode* node)
{
	// Remove from vector
	auto it = nodes.begin();
	for(; it != nodes.end(); it++)
	{
		if((*it) == node)
		{
			break;
		}
	}
	
	ANKI_ASSERT(it != nodes.end());
	nodes.erase(it);

	// Remove from dict
	if(node->getName())
	{
		auto it = dict.find(node->getName());

		ANKI_ASSERT(it != dict.end());
		dict.erase(it);
	}
}

//==============================================================================
SceneNode& SceneGraph::findSceneNode(const char* name)
{
	ANKI_ASSERT(dict.find(name) != dict.end());
	return *(dict.find(name))->second;
}

//==============================================================================
SceneNode* SceneGraph::tryFindSceneNode(const char* name)
{
	auto it = dict.find(name);
	return (it == dict.end()) ? nullptr : it->second;
}

//==============================================================================
void SceneGraph::deleteNodesMarkedForDeletion()
{
	/// Delete all nodes pending deletion. At this point all scene threads 
	/// should have finished their tasks
	while(nodesMarkedForDeletionCount > 0)
	{
		// First gather the nodes that will be de
		SceneFrameVector<decltype(nodes)::iterator> forDeletion;
		for(auto it = nodes.begin(); it != nodes.end(); it++)
		{
			if((*it)->isMarkedForDeletion())
			{
				forDeletion.push_back(it);
			}
		}

		// Now delete
		for(auto& it : forDeletion)
		{
			// Disable events for that node
			events.iterateEvents([&](Event& e)
			{
				if(e.getSceneNode() == *it)
				{
					e.markForDeletion();
				}
			});

			// Remove it
			unregisterNode(*it);

			SceneAllocator<SceneNode> al = alloc;
			alloc.destroy(*it);
			alloc.deallocate(*it, 1);

			++nodesMarkedForDeletionCount;
		}
	}
}

//==============================================================================
void SceneGraph::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
	ANKI_ASSERT(mainCam);

	ANKI_COUNTER_START_TIMER(C_SCENE_UPDATE_TIME);

	//
	// Sync point. Here we wait for all scene's threads
	//

	// Reset the framepool
	frameAlloc.reset();

	// Delete nodes
	deleteNodesMarkedForDeletion();

	// Sync updates
	iterateSceneNodes([&](SceneNode& node)
	{
		node.iterateComponents([&](SceneComponent& comp)
		{
			comp.reset();
			comp.updateReal(node, prevUpdateTime, crntTime, 
				SceneComponent::SYNC_UPDATE);
		});	
	});

	Threadpool& threadPool = ThreadpoolSingleton::get();
	(void)threadPool;

	// XXX Do that in parallel
	physics.update(prevUpdateTime, crntTime);
	renderer.getTiler().updateTiles(*mainCam);
	events.updateAllEvents(prevUpdateTime, crntTime);

	// Then the rest
	Array<UpdateSceneNodesJob, Threadpool::MAX_THREADS> jobs2;
	Barrier barrier(threadPool.getThreadsCount());

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		UpdateSceneNodesJob& job = jobs2[i];

		job.scene = this;
		job.prevUpdateTime = prevUpdateTime;
		job.crntTime = crntTime;
		job.barrier = &barrier;

		threadPool.assignNewTask(i, &job);
	}

	threadPool.waitForAllThreadsToFinish();

	doVisibilityTests(*mainCam, *this, renderer);

	ANKI_COUNTER_STOP_TIMER_INC(C_SCENE_UPDATE_TIME);
}

//==============================================================================
void SceneGraph::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(ANKI_R(filename));

		XmlElement rootEl = doc.getChildElement("scene");

		// Model nodes
		//
		XmlElement mdlNodeEl = rootEl.getChildElement("modelNode");

		do
		{
			XmlElement el, el1;
	
			// <name>
			el = mdlNodeEl.getChildElement("name");
			std::string name = el.getText();

			// <model>
			el = mdlNodeEl.getChildElement("model");

			// <instancesCount>
			el1 = mdlNodeEl.getChildElementOptional("instancesCount");
			U32 instancesCount = (el1) ? el1.getInt() : 1;

			if(instancesCount > ANKI_MAX_INSTANCES)
			{
				throw ANKI_EXCEPTION("Too many instances");
			}

			ModelNode* node;
			newSceneNode(node, name.c_str(), el.getText());

			// <transform>
			el = mdlNodeEl.getChildElement("transform");
			U i = 0;

			do
			{
				if(i == 0)
				{
					node->setLocalTransform(Transform(el.getMat4()));
				}
				else
				{
					InstanceNode* instance;
					std::string instName = name + "_inst" 
						+ std::to_string(i - 1);
					newSceneNode(instance, instName.c_str());

					instance->setLocalTransform(Transform(el.getMat4()));

					node->SceneNode::addChild(instance);
				}

				// Advance
				el = el.getNextSiblingElement("transform");
				++i;
			}
			while(el && i < instancesCount);

			if(i != instancesCount)
			{
				throw ANKI_EXCEPTION("instancesCount does not match "
					"with transform");
			}

			// Advance
			mdlNodeEl = mdlNodeEl.getNextSiblingElement("modelNode");
		} while(mdlNodeEl);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Scene loading failed") << e;
	}
}

} // end namespace anki
