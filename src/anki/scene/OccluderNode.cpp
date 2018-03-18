// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/OccluderNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/OccluderComponent.h>
#include <anki/resource/MeshLoader.h>

namespace anki
{

/// Feedback component.
class OccluderMoveFeedbackComponent : public SceneComponent
{
public:
	OccluderMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			OccluderNode& mnode = static_cast<OccluderNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

OccluderNode::~OccluderNode()
{
	m_vertsL.destroy(getSceneAllocator());
	m_vertsW.destroy(getSceneAllocator());
}

Error OccluderNode::init(const CString& meshFname)
{
	// Load mesh
	MeshLoader loader(&getSceneGraph().getResourceManager());
	ANKI_CHECK(loader.load(meshFname));

	const U indexCount = loader.getHeader().m_totalIndexCount;
	m_vertsL.create(getSceneAllocator(), indexCount);
	m_vertsW.create(getSceneAllocator(), indexCount);

	DynamicArrayAuto<Vec3> positions(getSceneAllocator());
	DynamicArrayAuto<U32> indices(getSceneAllocator());
	ANKI_CHECK(loader.storeIndicesAndPosition(indices, positions));

	for(U i = 0; i < indices.getSize(); ++i)
	{
		m_vertsL[i] = positions[indices[i]];
	}

	// Create the components
	newComponent<MoveComponent>(this);
	newComponent<OccluderMoveFeedbackComponent>(this);
	newComponent<OccluderComponent>(this);

	return Error::NONE;
}

void OccluderNode::onMoveComponentUpdate(MoveComponent& movec)
{
	const Transform& trf(movec.getWorldTransform());
	U count = m_vertsL.getSize();
	while(count--)
	{

		m_vertsW[count] = trf.transform(m_vertsL[count]);
	}

	getComponent<OccluderComponent>().setVertices(&m_vertsW[0], m_vertsW.getSize(), sizeof(m_vertsW[0]));
}

} // end namespace anki
