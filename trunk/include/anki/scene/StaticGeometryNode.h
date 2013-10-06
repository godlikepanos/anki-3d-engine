#ifndef ANKI_SCENE_STATIC_GEOMETRY_NODE_H
#define ANKI_SCENE_STATIC_GEOMETRY_NODE_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/RenderComponent.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Part of the static geometry. Used only for visibility tests
class StaticGeometrySpatial: public SpatialComponent
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// @note The node is only to steal the allocator
	StaticGeometrySpatial(const Obb* obb, SceneNode& node);
	/// @}
};

/// Static geometry scene node patch
class StaticGeometryPatchNode: public SceneNode, public SpatialComponent,
	public RenderComponent
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometryPatchNode(
		const char* name, SceneGraph* scene, // Scene
		const ModelPatchBase* modelPatch); // Self

	~StaticGeometryPatchNode();
	/// @}

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::getModelPatchBase
	const ModelPatchBase& getModelPatchBase()
	{
		return *modelPatch;
	}

	/// Implements  RenderComponent::getMaterial
	const Material& getMaterial()
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
