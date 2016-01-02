// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/RenderComponent.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Static geometry scene node patch
class StaticGeometryPatchNode: public SceneNode
{
	friend class StaticGeometryRenderComponent;

public:
	StaticGeometryPatchNode(SceneGraph* scene);

	~StaticGeometryPatchNode();

	ANKI_USE_RESULT Error create(
		const CString& name, const ModelPatch* modelPatch);

private:
	const ModelPatch* m_modelPatch;

	ANKI_USE_RESULT Error buildRendering(RenderingBuildInfo& data) const;
};

/// Static geometry scene node
class StaticGeometryNode: public SceneNode
{
public:
	StaticGeometryNode(SceneGraph* scene);

	~StaticGeometryNode();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& filename);

private:
	ModelResourcePtr m_model;
};
/// @}

} // end namespace anki

