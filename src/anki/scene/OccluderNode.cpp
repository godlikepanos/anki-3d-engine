// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/OccluderNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/OccluderComponent.h>
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

	ANKI_USE_RESULT Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			OccluderNode& mnode = static_cast<OccluderNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return ErrorCode::NONE;
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

	const U16* indices = reinterpret_cast<const U16*>(loader.getIndexData());
	U indexCount = loader.getIndexDataSize() / sizeof(U16);
	U vertSize = loader.getVertexSize();

	m_vertsL.create(getSceneAllocator(), indexCount);
	m_vertsW.create(getSceneAllocator(), indexCount);

	for(U i = 0; i < indexCount; ++i)
	{
		U idx = indices[i];
		const Vec3* vert = reinterpret_cast<const Vec3*>(loader.getVertexData() + idx * vertSize);
		m_vertsL[i] = *vert;
	}

	// Create the components
	SceneComponent* comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	comp = getSceneAllocator().newInstance<OccluderMoveFeedbackComponent>(this);
	addComponent(comp, true);

	comp = getSceneAllocator().newInstance<OccluderComponent>(this);
	addComponent(comp, true);

	return ErrorCode::NONE;
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
