#ifndef SKIN_NODE_H
#define SKIN_NODE_H

#include "SceneNode.h"
#include "SkinPatchNode.h"
#include "math/Math.h"
#include <boost/range/iterator_range.hpp>
#include <vector>


class Skin;
class SkelAnimModelNodeCtrl;


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
		RsrcPtr<Skin> skin; ///< The resource
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


#endif
