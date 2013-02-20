#ifndef ANKI_SCENE_STATIC_GEOMETRY_NODE_H
#define ANKI_SCENE_STATIC_GEOMETRY_NODE_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Part of the static geometry. Used only for visibility tests
class StaticGeometrySpatialNode: public SceneNode, public Spatial
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometrySpatialNode(const Obb& obb,
		const char* name, Scene* scene); // Scene
	/// @}
};

/// Static geometry scene node patch
class StaticGeometryPatchNode: public SceneNode, public Spatial,
	public Renderable
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometryPatchNode(const ModelPatchBase* modelPatch,
		const char* name, Scene* scene); // Scene

	~StaticGeometryPatchNode();
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase() const
	{
		return *modelPatch;
	}

	/// Implements  Renderable::getMaterial
	const Material& getRenderableMaterial() const
	{
		return modelPatch->getMaterial();
	}
	/// @}

private:
	const ModelPatchBase* modelPatch;
	SceneVector<StaticGeometrySpatialNode*> spatials;
};

/// Static geometry scene node
class StaticGeometryNode: public SceneNode
{
public:
	StaticGeometryNode(const char* filename,
		const char* name, Scene* scene); // Scene

	~StaticGeometryNode();

private:
	ModelResourcePointer model;
	SceneVector<StaticGeometryPatchNode*> patches;
};

/// @}

} // end namespace anki

#endif
