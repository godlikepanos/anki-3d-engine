#ifndef ANKI_SCENE_SKIN_NODE_H
#define ANKI_SCENE_SKIN_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/resource/Model.h"
#include "anki/math/Math.h"

namespace anki {

class Skin;

/// Skin specific mesh. It contains a number of VBOs for transform feedback
class SkinMesh: public MeshBase
{
public:
	/// Create the @a tfVbos with empty data
	SkinMesh(const MeshBase* mesh);

	/// @name Accessors
	/// @{
	const Vbo& getXfbVbo() const
	{
		return vbo;
	}
	/// @}

	/// @name MeshBase implementers
	/// @{
	U32 getVerticesCount() const
	{
		return mesh->getVerticesCount();
	}

	U32 getIndicesCount() const
	{
		return mesh->getIndicesCount();
	}

	U32 getTextureChannelsCount() const
	{
		return mesh->getTextureChannelsCount();
	}

	Bool hasWeights() const
	{
		return false;
	}

	const Obb& getBoundingShape() const
	{
		return mesh->getBoundingShape();
	}

	void getVboInfo(
		const VertexAttribute attrib, const Vbo*& vbo,
		U32& size, GLenum& type, U32& stride, U32& offset) const;
	/// @}

private:
	Vbo vbo; ///< Contains the transformed P,N,T 
	const MeshBase* mesh; ///< The resource
};


/// Skin specific ModelPatch. It uses a SkinMesh to create the VAOs. It also
/// creates a VAO for the transform feedback pass
class SkinModelPatch: public ModelPatchBase
{
public:
	/// See TfHwSkinningGeneric.glsl for the locations
	enum XfbAttributeLocation
	{
		XFBAL_POSITION,
		XFBAL_NORMAL,
		XFBAL_TANGENT,
		XFBAL_BONES_COUNT,
		XFBAL_BONE_IDS,
		XFBAL_BONE_WEIGHTS
	};

	/// @name Constructors/Destructor
	/// @{
	SkinModelPatch(const ModelPatch* mpatch_, 
		const SceneAllocator<U8>& alloc);
	~SkinModelPatch();
	/// @}

	/// @name Accessors
	/// @{
	SkinMesh& getSkinMesh(const PassLevelKey& key)
	{
		return *skinMeshes[key.level];
	}
	const SkinMesh& getSkinMesh(const PassLevelKey& key) const
	{
		return *skinMeshes[key.level];
	}

	const Vao& getTransformFeedbackVao(const PassLevelKey& key) const
	{
		return xfbVaos[key.level];
	}
	/// @}

	/// @name Implementations of ModelPatchBase virtuals
	/// @{
	const MeshBase& getMeshBase(const PassLevelKey& key) const
	{
		return *skinMeshes[key.level];
	}

	U32 getMeshesCount() const
	{
		return skinMeshes.size();
	}

	const Material& getMaterial() const
	{
		return mpatch->getMaterial();
	}
	/// @}

private:
	const ModelPatch* mpatch;
	SceneVector<SkinMesh*> skinMeshes;
	SceneVector<Vao> xfbVaos; ///< Used as a source VAO in XFB
};

/// A fragment of the SkinNode
class SkinPatchNode: public SceneNode, public Movable, public Renderable,
	public Spatial
{
public:
	/// @name Constructors/Destructor
	/// @{
	SkinPatchNode(const ModelPatch* modelPatch_,
		const char* name, Scene* scene, // Scene
		uint movableFlags, Movable* movParent, // Movable
		CollisionShape* spatialCs); // Spatial
	/// @}

	/// @name Accessors
	/// @{
	SkinModelPatch& getSkinModelPatch()
	{
		return *skinModelPatch;
	}
	const SkinModelPatch& getSkinModelPatch() const
	{
		return *skinModelPatch;
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::getSpatial()
	Spatial* getSpatial()
	{
		return this;
	}

	/// Override SceneNode::getRenderable
	Renderable* getRenderable()
	{
		return this;
	}
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase() const
	{
		return *skinModelPatch;
	}

	/// Implements Renderable::getMaterial
	const Material& getRenderableMaterial() const
	{
		return skinModelPatch->getMaterial();
	}

	/// Overrides Renderable::getRenderableWorldTransforms
	const Transform* getRenderableWorldTransforms() const
	{
		return &getWorldTransform();
	}

	/// Overrides Renderable::getRenderableOrigin
	Vec3 getRenderableOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

	/// @name Spatial virtuals
	/// @{

	/// Override Spatial::getSpatialOrigin
	const Vec3& getSpatialOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

private:
	std::unique_ptr<SkinModelPatch> skinModelPatch;
};

/// A skin scene node
class SkinNode: public SceneNode, public Movable
{
public:
	/// @name Constructors/Destructor
	/// @{
	SkinNode(const char* skinFname,
		const char* name, Scene* scene, // SceneNode
		uint movableFlags, Movable* movParent); // Movable

	~SkinNode();
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Update the animation stuff
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, int frame);
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Update boundingShapeWSpace from bone tails (not heads as well
	/// cause its faster that way). The tails come from the previous frame
	void movableUpdate();
	/// @}

	/// @name Accessors
	/// @{
	const SceneVector<Vec3>& getHeads() const
	{
		return heads;
	}

	const SceneVector<Vec3>& getTails() const
	{
		return tails;
	}

	const SceneVector<Mat3>& getBoneRotations() const
	{
		return boneRotations;
	}

	const SceneVector<Vec3>& getBoneTranslations() const
	{
		return boneTranslations;
	}

	const SceneVector<SkinPatchNode*>& getPatchNodes() const
	{
		return patches;
	}

	const Skin& getSkin() const
	{
		return *skin;
	}

	F32 getStep() const
	{
		return step;
	}
	F32& getStep()
	{
		return step;
	}
	void setStep(const F32 x)
	{
		step = x;
	}

	F32 getFrame() const
	{
		return frame;
	}
	F32& getFrame()
	{
		return frame;
	}
	void setFrame(const F32 x)
	{
		frame = x;
	}

	void setAnimation(const SkelAnim& anim_)
	{
		anim = &anim_;
	}
	const SkelAnim* getAnimation() const
	{
		return anim;
	}
	/// @}

private:
	SkinResourcePointer skin; ///< The resource
	SceneVector<SkinPatchNode*> patches;
	Obb visibilityShapeWSpace;

	/// @name Animation stuff
	/// @{
	F32 step;
	F32 frame;
	const SkelAnim* anim; ///< The active skeleton animation
	/// @}

	/// @name Bone data
	/// @{
	SceneVector<Vec3> heads;
	SceneVector<Vec3> tails;
	SceneVector<Mat3> boneRotations;
	SceneVector<Vec3> boneTranslations;
	/// @}

	/// Interpolate
	/// @param[in] animation Animation
	/// @param[in] frame Frame
	/// @param[out] translations Translations vector
	/// @param[out] rotations Rotations vector
	static void interpolate(const SkelAnim& animation, F32 frame,
		SceneVector<Vec3>& translations, SceneVector<Mat3>& rotations);

	/// Calculate the global pose
	static void updateBoneTransforms(const Skeleton& skel,
		SceneVector<Vec3>& translations, SceneVector<Mat3>& rotations);

	/// Deform the heads and tails
	static void deformHeadsTails(const Skeleton& skeleton,
		const SceneVector<Vec3>& boneTranslations,
		const SceneVector<Mat3>& boneRotations,
		SceneVector<Vec3>& heads, SceneVector<Vec3>& tails);
};

} // end namespace

#endif
