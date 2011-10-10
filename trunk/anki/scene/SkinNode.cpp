#include "anki/scene/SkinNode.h"
#include "anki/resource/Skin.h"
#include "anki/scene/SkinPatchNode.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/SkelAnim.h"
#include <boost/foreach.hpp>


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SkinNode::SkinNode(Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(SNT_SKIN_NODE, scene, flags, parent)
{}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
SkinNode::~SkinNode()
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void SkinNode::init(const char* filename)
{
	skin.loadRsrc(filename);

	BOOST_FOREACH(const ModelPatch& patch, skin->getModelPatches())
	{
		patches.push_back(new SkinPatchNode(patch, *this));
	}

	uint bonesNum = skin->getSkeleton().getBones().size();
	tails.resize(bonesNum);
	heads.resize(bonesNum);
	boneRotations.resize(bonesNum);
	boneTranslations.resize(bonesNum);
}


//==============================================================================
// moveUpdate                                                                  =
//==============================================================================
void SkinNode::moveUpdate()
{
	visibilityShapeWSpace.set(tails);
	visibilityShapeWSpace = visibilityShapeWSpace.getTransformed(
		getWorldTransform());
}


//==============================================================================
// frameUpdate                                                                 =
//==============================================================================
void SkinNode::frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/)
{
	frame += step;

	if(frame > anim->getFramesNum())
	{
		frame = 0.0;
	}

	// A nasty optimization that may produse ugly results
	if(!isFlagEnabled(SNF_VISIBLE))
	{
		return;
	}

	interpolate(*anim, frame, boneTranslations,
	    boneRotations);

	updateBoneTransforms(getSkin().getSkeleton(),
	    boneTranslations, boneRotations);

	deformHeadsTails(getSkin().getSkeleton(), boneTranslations,
	    boneRotations, heads, tails);
}


//==============================================================================
// interpolate                                                                 =
//==============================================================================
void SkinNode::interpolate(const SkelAnim& animation, float frame,
	std::vector<Vec3>& boneTranslations, std::vector<Mat3>& boneRotations)
{
	ASSERT(frame < animation.getFramesNum());

	// calculate the t (used in slerp and lerp) using the keyframs and the
	// frame and calc the lPose and rPose witch indicate the pose IDs in witch
	// the frame lies between
	const std::vector<uint>& keyframes = animation.getKeyframes();
	float t = 0.0;
	uint lPose = 0, rPose = 0;
	for(uint j = 0; j < keyframes.size(); j++)
	{
		if((float)keyframes[j] == frame)
		{
			lPose = rPose = j;
			t = 0.0;
			break;
		}
		else if((float)keyframes[j] > frame)
		{
			lPose = j-1;
			rPose = j;
			t = (frame - (float)keyframes[lPose]) / float(keyframes[rPose] -
				keyframes[lPose]);
			break;
		}
	}

	// now for all bones update bone's poses
	ASSERT(boneRotations.size() >= 1);
	for(uint i=0; i < boneRotations.size(); i++)
	{
		const BoneAnim& banim = animation.getBoneAnimations()[i];

		Mat3& localRot = boneRotations[i];
		Vec3& localTransl = boneTranslations[i];

		// if the bone has animations then slerp and lerp to find the rotation
		// and translation
		if(banim.getBonePoses().size() != 0)
		{
			const BonePose& lBpose = banim.getBonePoses()[lPose];
			const BonePose& rBpose = banim.getBonePoses()[rPose];

			// rotation
			const Quat& q0 = lBpose.getRotation();
			const Quat& q1 = rBpose.getRotation();
			localRot = Mat3(q0.slerp(q1, t));

			// translation
			const Vec3& v0 = lBpose.getTranslation();
			const Vec3& v1 = rBpose.getTranslation();
			localTransl = v0.lerp(v1, t);
		}
		// else put the idents
		else
		{
			localRot = Mat3::getIdentity();
			localTransl = Vec3(0.0, 0.0, 0.0);
		}
	}
}


//==============================================================================
// updateBoneTransforms                                                        =
//==============================================================================
void SkinNode::updateBoneTransforms(const Skeleton& skeleton,
	std::vector<Vec3>& boneTranslations, std::vector<Mat3>& boneRotations)
{
	boost::array<uint, 128> queue;
	uint head = 0, tail = 0;

	// put the roots
	BOOST_FOREACH(const Bone& bone, skeleton.getBones())
	{
		if(bone.getParent() == NULL)
		{
			queue[tail++] = bone.getId(); // queue push
		}
	}

	// loop
	while(head != tail) // while queue not empty
	{
		uint boneId = queue[head++]; // queue pop
		const Bone& boned = skeleton.getBones()[boneId];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated
		// transformation.
		Math::combineTransformations(
			boneTranslations[boneId], boneRotations[boneId],
			boned.getTranslationSkeletonSpaceInverted(),
			boned.getRotationSkeletonSpaceInverted(),
			boneTranslations[boneId], boneRotations[boneId]);

		Math::combineTransformations(
			boned.getTranslationSkeletonSpace(),
			boned.getRotationSkeletonSpace(),
			boneTranslations[boneId], boneRotations[boneId],
			boneTranslations[boneId], boneRotations[boneId]);

		// and finaly add the parent's transform
		if(boned.getParent())
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			Math::combineTransformations(
				boneTranslations[boned.getParent()->getId()],
				boneRotations[boned.getParent()->getId()],
				boneTranslations[boneId],
				boneRotations[boneId],
				boneTranslations[boneId],
				boneRotations[boneId]);
		}

		// now add the bone's children
		for(uint i = 0; i < boned.getChildsNum(); i++)
		{
			queue[tail++] = boned.getChild(i).getId();
		}
	}
}


//==============================================================================
// deformHeadsTails                                                            =
//==============================================================================
void SkinNode::deformHeadsTails(const Skeleton& skeleton,
    const std::vector<Vec3>& boneTranslations,
    const std::vector<Mat3>& boneRotations,
    std::vector<Vec3>& heads, std::vector<Vec3>& tails)
{
	for(uint i = 0; i < skeleton.getBones().size(); i++)
	{
		const Mat3& rot = boneRotations[i];
		const Vec3& transl = boneTranslations[i];

		heads[i] = skeleton.getBones()[i].getHead().getTransformed(transl, rot);
		tails[i] = skeleton.getBones()[i].getTail().getTransformed(transl, rot);
	}
}
