// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Math.h>

namespace anki {

// Forward
class SkyboxQueueElement;

/// @addtogroup scene
/// @{

/// @memberof SkyboxComponent
enum class SkyboxType : U8
{
	SOLID_COLOR,
	IMAGE_2D
};

/// Skybox config.
class SkyboxComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SkyboxComponent)

public:
	SkyboxComponent(SceneNode* node);

	~SkyboxComponent();

	void setSolidColor(const Vec3& color)
	{
		m_type = SkyboxType::SOLID_COLOR;
		m_color = color.max(Vec3(0.0f));
	}

	void setImage(CString filename);

	void setupSkyboxQueueElement(SkyboxQueueElement& queueElement) const;

private:
	SceneNode* m_node;
	SkyboxType m_type = SkyboxType::SOLID_COLOR;
	Vec3 m_color = Vec3(0.0f, 0.0f, 0.5f);
	ImageResourcePtr m_image;
};
/// @}

} // end namespace anki
