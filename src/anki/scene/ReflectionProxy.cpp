// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProxy.h>
#include <anki/scene/ReflectionProxyComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/resource/MeshLoader.h>

namespace anki
{

/// Feedback component
class ReflectionProxyMoveFeedbackComponent : public SceneComponent
{
public:
	ReflectionProxyMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	Error update(SceneNode& node, F32, F32, Bool& updated) final
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProxy& dnode = static_cast<ReflectionProxy&>(node);
			dnode.onMoveUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

Error ReflectionProxy::init(const CString& proxyMesh)
{
	// Move component first
	SceneComponent* comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<ReflectionProxyMoveFeedbackComponent>(this);
	addComponent(comp, true);

	// Load vertices
	MeshLoader loader(&getResourceManager());
	ANKI_CHECK(loader.load(proxyMesh));

	if((loader.getHeader().m_flags & MeshLoader::Flag::QUADS) == MeshLoader::Flag::NONE)
	{
		ANKI_SCENE_LOGE("Expecting quad mesh");
		return ErrorCode::USER_DATA;
	}

	const U indexCount = loader.getHeader().m_totalIndicesCount;
	const U quadCount = indexCount / 4;
	m_quadsLSpace.create(getSceneAllocator(), quadCount);

	const U8* buff = loader.getVertexData();
	const U8* buffEnd = loader.getVertexData() + loader.getVertexDataSize();
	WeakArray<const U16> indices(reinterpret_cast<const U16*>(loader.getIndexData()), indexCount);
	for(U i = 0; i < quadCount; ++i)
	{
		Array<Vec4, 4>& quad = m_quadsLSpace[i];

		for(U j = 0; j < 4; ++j)
		{
			U index = indices[i * 4 + j];

			const Vec3* vert = reinterpret_cast<const Vec3*>(buff + loader.getVertexSize() * index);
			ANKI_ASSERT(vert + 1 < reinterpret_cast<const Vec3*>(buffEnd));
			(void)buffEnd;

			quad[j] = vert->xyz0();
		}
	}

	// Proxy component
	comp = getSceneAllocator().newInstance<ReflectionProxyComponent>(this, quadCount);
	addComponent(comp, true);

	// Spatial component
	m_boxLSpace.setFromPointCloud(loader.getVertexData(),
		loader.getHeader().m_totalVerticesCount,
		loader.getVertexSize(),
		loader.getVertexDataSize());

	m_boxWSpace = m_boxLSpace;

	comp = getSceneAllocator().newInstance<SpatialComponent>(this, &m_boxWSpace);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

void ReflectionProxy::onMoveUpdate(const MoveComponent& move)
{
	const Transform& trf = move.getWorldTransform();

	// Update proxy comp
	ReflectionProxyComponent& proxyc = getComponent<ReflectionProxyComponent>();
	for(U i = 0; i < m_quadsLSpace.getSize(); ++i)
	{
		Array<Vec4, 4> quadWSpace;
		for(U j = 0; j < 4; ++j)
		{
			quadWSpace[j] = trf.transform(m_quadsLSpace[i][j]);
		}

		proxyc.setQuad(i, quadWSpace[0], quadWSpace[1], quadWSpace[2], quadWSpace[3]);
	}

	// Update spatial
	m_boxWSpace = m_boxLSpace;
	m_boxWSpace.transform(trf);
	SpatialComponent& spatial = getComponent<SpatialComponent>();
	spatial.setSpatialOrigin(move.getWorldTransform().getOrigin());
	spatial.markForUpdate();
}

} // end namespace anki
