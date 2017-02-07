// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

	// Add a border to the UVs to avoid complex shader logic
	if(l.m_atlas->getSubTextureMargin() < ATLAS_SUB_TEXTURE_MARGIN)
	{
		ANKI_SCENE_LOGE("Need texture atlas with margin at least %u", ATLAS_SUB_TEXTURE_MARGIN);
		return ErrorCode::USER_DATA;
	}

	Vec2 marginf = F32(ATLAS_SUB_TEXTURE_MARGIN / 2) / Vec2(l.m_atlas->getWidth(), l.m_atlas->getHeight());
	Vec2 minUv = l.m_uv.xy() - marginf;
	Vec2 sizeUv = (l.m_uv.zw() - l.m_uv.xy()) + 2.0f * marginf;
	l.m_uv = Vec4(minUv.x(), minUv.y(), minUv.x() + sizeUv.x(), minUv.y() + sizeUv.y());

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
