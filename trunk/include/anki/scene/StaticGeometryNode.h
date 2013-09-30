#ifndef ANKI_SCENE_STATIC_GEOMETRY_NODE_H
#define ANKI_SCENE_STATIC_GEOMETRY_NODE_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/Renderable.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Part of the static geometry. Used only for visibility tests
class StaticGeometrySpatial: public SpatialComponent
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometrySpatial(const Obb* obb, const SceneAllocator<U8>& alloc);
	/// @}
};

/// Static geometry scene node patch
class StaticGeometryPatchNode: public SceneNode, public SpatialComponent,
	public Renderable
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometryPatchNode(
		const char* name, SceneGraph* scene, // Scene
		const ModelPatchBase* modelPatch); // Self

	~StaticGeometryPatchNode();
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase()
	{
		return *modelPatch;
	}

	/// Implements  Renderable::getMaterial
	const Material& getRenderableMaterial()
	{
		return modelPatch->getMaterial();
	}
	/// @}

private:
	const ModelPatchBase* modelPatch;
};

/// Static geometry scene node
class StaticGeometryNode: public SceneNode
{
public:
	StaticGeometryNode(
		const char* name, SceneGraph* scene, // Scene
		const char* filename); // Self

	~StaticGeometryNode();

private:
	ModelResourcePointer model;
	SceneVector<StaticGeometryPatchNode*> patches;
};

/// @}

} // end namespace anki

#endif
