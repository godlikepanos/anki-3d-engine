// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/RenderComponent.h>

namespace anki
{

// Forward
class ModelPatch;

/// @addtogroup scene
/// @{

/// Static geometry scene node patch
class StaticGeometryPatchNode : public SceneNode
{
	friend class StaticGeometryRenderComponent;

public:
	StaticGeometryPatchNode(SceneGraph* scene, CString name);

	~StaticGeometryPatchNode();

	ANKI_USE_RESULT Error init(const ModelPatch* modelPatch);

private:
	const ModelPatch* m_modelPatch;

	ANKI_USE_RESULT Error buildRendering(const RenderingBuildInfoIn& in, RenderingBuildInfoOut& out) const;
};

/// Static geometry scene node
class StaticGeometryNode : public SceneNode
{
public:
	StaticGeometryNode(SceneGraph* scene, CString name);

	~StaticGeometryNode();

	ANKI_USE_RESULT Error init(const CString& filename);

private:
	ModelResourcePtr m_model;
};
/// @}

} // end namespace anki
