#ifndef ANKI_SCENE_SKIN_NODE_H
#define ANKI_SCENE_SKIN_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/resource/Model.h"
#include "anki/math/Math.h"
#include "anki/util/Vector.h"
#include <array>

namespace anki {

class Skin;

/// Skin specific mesh. It contains a number of VBOs for transform feedback
class SkinMesh: public MeshBase
{
public:
	/// Transform feedback VBOs
	enum TfVboId
	{
		VBO_TF_POSITIONS, ///< VBO never empty
		VBO_TF_NORMALS, ///< VBO never empty
		VBO_TF_TANGENTS, ///< VBO never empty
		VBOS_TF_COUNT
	};

	/// Create the @a tfVbos with empty data
	SkinMesh(const MeshBase* mesh);

	/// @name Accessors
	/// @{
	const Vbo* getTransformFeedbackVbo(TfVboId id) const /// XXX Why pointer?
	{
		return &tfVbos[id];
	}
	/// @}

	/// @name Implementations of MeshBase virtuals
	/// @{
	const Vbo* getVbo(VboId id) const;

	uint getTextureChannelsNumber() const
	{
		return mesh->getTextureChannelsNumber();
	}

	uint getLodsNumber() const
	{
		return mesh->getLodsNumber();
	}

	uint getIndicesNumber(uint lod) const
	{
		return mesh->getIndicesNumber(lod);
	}

	uint getVerticesNumber(uint lod) const
	{
		return mesh->getVerticesNumber(lod);
	}

	const Obb& getBoundingShape() const
	{
		return mesh->getBoundingShape();
	}
	/// @}

private:
	std::array<Vbo, VBOS_TF_COUNT> tfVbos;
	const MeshBase* mesh; ///< The resource
};


/// Skin specific ModelPatch. It uses a SkinMesh to create the VAOs. It also
/// creates a VAO for the transform feedback pass
class SkinModelPatch: public ModelPatchBase
{
public:
	/// See TfHwSkinningGeneric.glsl for the locations
	enum TfShaderProgAttribLoc
	{
		POSITION_LOC,
		NORMAL_LOC,
		TANGENT_LOC,
		VERT_WEIGHT_BONES_NUM_LOC,
		VERT_WEIGHT_BONE_IDS_LOC,
		VERT_WEIGHT_WEIGHTS_LOC
	};

	/// @name Constructors/Destructor
	/// @{
	SkinModelPatch(const ModelPatch* mpatch_);
	/// @}

	/// @name Accessors
	/// @{
	SkinMesh& getSkinMesh()
	{
		return *skinMesh;
	}

	const SkinMesh& getSkinMesh() const
	{
		return *skinMesh;
	}

	const Vao& getTransformFeedbackVao() const
	{
		return tfVao;
	}
	/// @}

	/// @name Implementations of ModelPatchBase virtuals
	/// @{
	const MeshBase& getMeshBase() const
	{
		return *skinMesh;
	}

	const Material& getMaterial() const
	{
		return mpatch->getMaterial();
	}
	/// @}

private:
	std::unique_ptr<SkinMesh> skinMesh;
	const ModelPatch* mpatch;
	Vao tfVao; ///< Used as a source VAO in TFB
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
	const ModelPatchBase& getModelPatchBase() const
	{
		return *skinModelPatch;
	}

	/// Implements Renderable::getMaterial
	const Material& getMaterial() const
	{
		return skinModelPatch->getMaterial();
	}

	/// Overrides Renderable::getRenderableWorldTransform
	const Transform* getRenderableWorldTransform() const
	{
		return &getWorldTransform();
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
	void frameUpdate(float prevUpdateTime, float crntTime, int frame);
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Update boundingShapeWSpace from bone tails (not heads as well
	/// cause its faster that way). The tails come from the previous frame
	void movableUpdate();
	/// @}

	/// @name Accessors
	/// @{
	const Vector<Vec3>& getHeads() const
	{
		return heads;
	}

	const Vector<Vec3>& getTails() const
	{
		return tails;
	}

	const Vector<Mat3>& getBoneRotations() const
	{
		return boneRotations;
	}

	const Vector<Vec3>& getBoneTranslations() const
	{
		return boneTranslations;
	}

	const PtrVector<SkinPatchNode>& getPatchNodes() const
	{
		return patches;
	}

	const Skin& getSkin() const
	{
		return *skin;
	}

	float getStep() const
	{
		return step;
	}
	float& getStep()
	{
		return step;
	}
	void setStep(const float x)
	{
		step = x;
	}

	float getFrame() const
	{
		return frame;
	}
	float& getFrame()
	{
		return frame;
	}
	void setFrame(const float x)
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
	PtrVector<SkinPatchNode> patches;
	Obb visibilityShapeWSpace;

	/// @name Animation stuff
	/// @{
	float step;
	float frame;
	const SkelAnim* anim; ///< The active skeleton animation
	/// @}

	/// @name Bone data
	/// @{
	Vector<Vec3> heads;
	Vector<Vec3> tails;
	Vector<Mat3> boneRotations;
	Vector<Vec3> boneTranslations;
	/// @}

	/// Interpolate
	/// @param[in] animation Animation
	/// @param[in] frame Frame
	/// @param[out] translations Translations vector
	/// @param[out] rotations Rotations vector
	static void interpolate(const SkelAnim& animation, float frame,
		Vector<Vec3>& translations, std::vector<Mat3>& rotations);

	/// Calculate the global pose
	static void updateBoneTransforms(const Skeleton& skel,
		Vector<Vec3>& translations, std::vector<Mat3>& rotations);

	/// Deform the heads and tails
	static void deformHeadsTails(const Skeleton& skeleton,
		const Vector<Vec3>& boneTranslations,
		const Vector<Mat3>& boneRotations,
		Vector<Vec3>& heads, std::vector<Vec3>& tails);
};

} // end namespace

#endif
