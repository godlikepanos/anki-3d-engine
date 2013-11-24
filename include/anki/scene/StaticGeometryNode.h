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
	StaticGeometrySpatial(SceneNode* node, const Obb* obb);

	const CollisionShape& getSpatialCollisionShape()
	{
		return *obb;
	}

	Vec3 getSpatialOrigin()
	{
		return obb->getCenter();
	}

private:
	const Obb* obb;
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

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return *obb;
	}

	Vec3 getSpatialOrigin()
	{
		return obb->getCenter();
	}
	/// @}

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::getRenderingData
	void getRenderingData(
		const PassLodKey& key, 
		const U8* subMeshIndicesArray, U subMeshIndicesCount,
		const Vao*& vao, const ShaderProgram*& prog,
		Drawcall& drawcall);

	/// Implements  RenderComponent::getMaterial
	const Material& getMaterial()
	{
		return modelPatch->getMaterial();
	}
	/// @}

private:
	const ModelPatchBase* modelPatch;
	const Obb* obb; 
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
};

/// @}

} // end namespace anki

#endif
