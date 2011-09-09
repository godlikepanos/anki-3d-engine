#include "SkinNode.h"
#include "rsrc/Skin.h"
#include "SkinPatchNode.h"
#include "rsrc/Skeleton.h"
#include "rsrc/SkelAnim.h"
#include <boost/foreach.hpp>


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SkinNode::SkinNode(bool inheritParentTrfFlag, SceneNode* parent)
:	SceneNode(SNT_SKIN, inheritParentTrfFlag, parent)
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
		patches.push_back(new SkinPatchNode(patch, this));
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
void SkinNode::frameUpdate(float prevUpdateTime, float /*crntTime*/)
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

	interpolate(*anim, frame, getBoneTranslations(),
	    getBoneRotations());

	updateBoneTransforms(getSkin().getSkeleton(),
	    getBoneTranslations(), getBoneRotations());

	deformHeadsTails(getSkin().getSkeleton(), getBoneTranslations(),
	    getBoneRotations(), getHeads(), getTails());
}


//==============================================================================
// interpolate                                                                 =
//==============================================================================
void SkinNode::interpolate(const SkelAnim& animation, float frame,
	Vec<Vec3>& boneTranslations, Vec<Mat3>& boneRotations)
{
	ASSERT(frame < animation.getFramesNum());

	// calculate the t (used in slerp and lerp) using the keyframs and the
	// frame and calc the lPose and rPose witch indicate the pose IDs in witch
	// the frame lies between
	const Vec<uint>& keyframes = animation.getKeyframes();
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
		const BoneAnim& banim = animation.getBoneAnims()[i];

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
	Vec<Vec3>& boneTranslations, Vec<Mat3>& boneRotations)
{
	boost::array<uint, 128> queue;
	uint head = 0, tail = 0;

	// put the roots
	BOOST_FOREACH(const Bone& bone, skeleton.getBones())
	{
		if(bone.getParent() == NULL)
		{
			queue[tail++] = bone.getPos(); // queue push
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
			boned.getTslSkelSpaceInv(), boned.getRotSkelSpaceInv(),
			boneTranslations[boneId], boneRotations[boneId]);

		Math::combineTransformations(
			boned.getTslSkelSpace(), boned.getRotSkelSpace(),
			boneTranslations[boneId], boneRotations[boneId],
			boneTranslations[boneId], boneRotations[boneId]);

		// and finaly add the parent's transform
		if(boned.getParent())
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			Math::combineTransformations(
				boneTranslations[boned.getParent()->getPos()],
				boneRotations[boned.getParent()->getPos()],
				boneTranslations[boneId],
				boneRotations[boneId],
				boneTranslations[boneId],
				boneRotations[boneId]);
		}

		// now add the bone's children
		for(uint i = 0; i < boned.getChildsNum(); i++)
		{
			queue[tail++] = boned.getChild(i).getPos();
		}
	}
}


//==============================================================================
// deformHeadsTails                                                            =
//==============================================================================
void SkinNode::deformHeadsTails(const Skeleton& skeleton,
    const Vec<Vec3>& boneTranslations, const Vec<Mat3>& boneRotations,
    Vec<Vec3>& heads, Vec<Vec3>& tails)
{
	for(uint i = 0; i < skeleton.getBones().size(); i++)
	{
		const Mat3& rot = boneRotations[i];
		const Vec3& transl = boneTranslations[i];

		heads[i] = skeleton.getBones()[i].getHead().getTransformed(transl, rot);
		tails[i] = skeleton.getBones()[i].getTail().getTransformed(transl, rot);
	}
}
