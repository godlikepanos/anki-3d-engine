// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/DecalComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

//==============================================================================
DecalComponent::~DecalComponent()
{
}

//==============================================================================
Error DecalComponent::setLayer(CString texAtlasFname,
	CString texAtlasSubtexName,
	F32 blendFactor,
	LayerType type)
{
	Layer& l = m_layers[type];

	ANKI_CHECK(getSceneGraph().getResourceManager().loadResource(
		texAtlasFname, l.m_atlas));

	ANKI_CHECK(l.m_atlas->getSubTextureInfo(texAtlasSubtexName, &l.m_uv[0]));
	l.m_blendFactor = blendFactor;

	return ErrorCode::NONE;
}

//==============================================================================
void DecalComponent::updateMatrices(const Obb& box)
{
	Mat4 worldTransform(
		box.getCenter().xyz1(), box.getRotation().getRotationPart(), 1.0f);

	Mat4 viewMat = worldTransform.getInverse();

	const Vec4& extend = box.getExtend();
	Mat4 projMat = Mat4::calculateOrthographicProjectionMatrix(extend.x(),
		-extend.x(),
		extend.y(),
		-extend.y(),
		FRUSTUM_NEAR_PLANE,
		extend.z() * 2.0);

	static const Mat4 biasMat4(0.5,
		0.0,
		0.0,
		0.5,
		0.0,
		0.5,
		0.0,
		0.5,
		0.0,
		0.0,
		0.5,
		0.5,
		0.0,
		0.0,
		0.0,
		1.0);

	m_biasProjViewMat = biasMat4 * projMat * viewMat;
}

} // end namespace anki
