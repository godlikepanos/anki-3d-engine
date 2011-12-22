#ifndef ANKI_SCENE_SKIN_NODE_H
#define ANKI_SCENE_SKIN_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/math/Math.h"
#include <boost/range/iterator_range.hpp>
#include <vector>


namespace anki {


class Skin;


/// A fragment of the SkinNode
class SkinPatchNode: public Renderable, public SceneNode
{
	public:
		enum TransformFeedbackVbo
		{
			TFV_POSITIONS,
			TFV_NORMALS,
			TFV_TANGENTS,
			TFV_NUM
		};

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

		SkinPatchNode(const ModelPatch* modelPatch, SkinNode* parent)
			: SceneNode(SNT_RENDERABLE_NODE, parent->getScene(),
				SNF_INHERIT_PARENT_TRANSFORM, parent), modelPatch(modelPatch_)
		{
			mtlr.reset(new MaterialRuntime(modelPatch->getMaterial()));
		}

		/// @name Accessors
		/// @{
		const Vao& getTfVao() const
		{
			return tfVao;
		}

		const Vbo& getTfVbo(uint i) const
		{
			return tfVbos[i];
		}

		const Vao& getVao(const PassLevelKey& k) const
		{
			return *vaosHashMap.at(k);
		}
		/// @}

		void init(const char*)
		{}

		/// Implements Renderable::getVao
		const Vao& getVao(const PassLevelKey& k)
		{
			return vaosHashMap[k];
		}

		/// Implements Renderable::getVertexIdsNum
		uint getVertexIdsNum(const PassLevelKey& k)
		{
			return modelPatch->getMesh().getVertsNum();
		}

		/// Implements Renderable::getMaterialRuntime
		MaterialRuntime& getMaterialRuntime()
		{
			return *mtlr;
		}

		/// Implements Renderable::getWorldTransform
		const Transform& getWorldTransform(const PassLevelKey&)
		{
			return SceneNode::getWorldTransform();
		}

		/// Implements Renderable::getPreviousWorldTransform
		const Transform& getPreviousWorldTransform(
			const PassLevelKey& k)
		{
			return SceneNode::getPrevWorldTransform();
		}

	private:
		ModelPatch::VaosContainer vaos;
		ModelPatch::PassLevelToVaoHashMap vaosHashMap;
		const ModelPatch* modelPatch;
		boost::scoped_ptr<MaterialRuntime> mtlr; ///< Material runtime

		boost::array<Vbo, TFV_NUM> tfVbos;
		Vao tfVao; ///< For TF passes
};


/// A skin scene node
class SkinNode: public SceneNode
{
	public:
		template<typename T>
		class Types
		{
			public:
				typedef std::vector<T> Container;
				typedef typename Container::iterator Iterator;
				typedef typename Container::const_iterator ConstIterator;
				typedef boost::iterator_range<Iterator> MutableRange;
				typedef boost::iterator_range<ConstIterator> ConstRange;
		};

		SkinNode(Scene& scene, ulong flags, SceneNode* parent);
		~SkinNode();

		/// @name Accessors
		/// @{
		Types<Vec3>::ConstRange getHeads() const
		{
			return Types<Vec3>::ConstRange(heads.begin(), heads.end());
		}
		Types<Vec3>::MutableRange getHeads()
		{
			return Types<Vec3>::MutableRange(heads.begin(), heads.end());
		}

		Types<Vec3>::ConstRange getTails() const
		{
			return Types<Vec3>::ConstRange(tails.begin(), tails.end());
		}
		Types<Vec3>::MutableRange getTails()
		{
			return Types<Vec3>::MutableRange(tails.begin(), tails.end());
		}

		Types<Mat3>::ConstRange getBoneRotations() const
		{
			return Types<Mat3>::ConstRange(boneRotations.begin(),
				boneRotations.end());
		}
		Types<Mat3>::MutableRange getBoneRotations()
		{
			return Types<Mat3>::MutableRange(boneRotations.begin(),
				boneRotations.end());
		}

		Types<Vec3>::ConstRange getBoneTranslations() const
		{
			return Types<Vec3>::ConstRange(boneTranslations.begin(),
				boneTranslations.end());
		}
		Types<Vec3>::MutableRange getBoneTranslations()
		{
			return Types<Vec3>::MutableRange(boneTranslations.begin(),
				boneTranslations.end());
		}

		Types<SkinPatchNode*>::ConstRange getPatchNodes() const
		{
			return Types<SkinPatchNode*>::ConstRange(patches.begin(),
				patches.end());
		}
		Types<SkinPatchNode*>::MutableRange getPatchNodes()
		{
			return Types<SkinPatchNode*>::MutableRange(patches.begin(),
				patches.end());
		}

		const CollisionShape*
			getVisibilityCollisionShapeWorldSpace() const
		{
			return &visibilityShapeWSpace;
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

		void init(const char* filename);

		/// Update boundingShapeWSpace from bone tails (not heads as well
		/// cause its faster that way). The tails come from the previous frame
		void moveUpdate();

		/// Update the animation stuff
		void frameUpdate(float prevUpdateTime, float crntTime);

	private:
		SkinResourcePointer skin; ///< The resource
		std::vector<SkinPatchNode*> patches;
		Obb visibilityShapeWSpace;

		/// @name Animation stuff
		/// @{
		float step;
		float frame;
		const SkelAnim* anim; ///< The active skeleton animation
		/// @}

		/// @name Bone data
		/// @{
		std::vector<Vec3> heads;
		std::vector<Vec3> tails;
		std::vector<Mat3> boneRotations;
		std::vector<Vec3> boneTranslations;
		/// @}

		/// Interpolate
		/// @param[in] animation Animation
		/// @param[in] frame Frame
		/// @param[out] translations Translations vector
		/// @param[out] rotations Rotations vector
		static void interpolate(const SkelAnim& animation, float frame,
			std::vector<Vec3>& translations, std::vector<Mat3>& rotations);

		/// Calculate the global pose
		static void updateBoneTransforms(const Skeleton& skel,
			std::vector<Vec3>& translations, std::vector<Mat3>& rotations);

		/// Deform the heads and tails
		static void deformHeadsTails(const Skeleton& skeleton,
		    const std::vector<Vec3>& boneTranslations,
		    const std::vector<Mat3>& boneRotations,
		    std::vector<Vec3>& heads, std::vector<Vec3>& tails);
};


} // end namespace


#endif
