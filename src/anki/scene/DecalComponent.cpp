// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/DecalComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

DecalComponent::DecalComponent(SceneNode* node)
	: SceneComponent(CLASS_TYPE, node)
{
}

DecalComponent::~DecalComponent()
{
}

Error DecalComponent::setLayer(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor, LayerType type)
{
	Layer& l = m_layers[type];

	ANKI_CHECK(getSceneGraph().getResourceManager().loadResource(texAtlasFname, l.m_atlas));

	ANKI_CHECK(l.m_atlas->getSubTextureInfo(texAtlasSubtexName, &l.m_uv[0]));
	l.m_blendFactor = blendFactor;

	return ErrorCode::NONE;
}

void DecalComponent::updateInternal()
{
	Mat4 worldTransform(m_trf);

	Mat4 viewMat = worldTransform.getInverse();

	Mat4 projMat = Mat4::calculateOrthographicProjectionMatrix(m_sizes.x() / 2.0f,
		-m_sizes.x() / 2.0f,
		m_sizes.y() / 2.0f,
		-m_sizes.y() / 2.0f,
		FRUSTUM_NEAR_PLANE,
		m_sizes.z());

	static const Mat4 biasMat4(0.5, 0.0, 0.0, 0.5, 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);

	m_biasProjViewMat = biasMat4 * projMat * viewMat;
}

} // end namespace anki
