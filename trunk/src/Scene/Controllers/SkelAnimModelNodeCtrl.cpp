#include <boost/foreach.hpp>
#include "SkelAnimModelNodeCtrl.h"
#include "SkelAnim.h"
#include "Skeleton.h"
#include "SkinNode.h"
#include "Model.h"
#include "MainRenderer.h"
#include "Skin.h"


//======================================================================================================================
// SkelAnimModelNodeCtrl                                                                                               =
//======================================================================================================================
SkelAnimModelNodeCtrl::SkelAnimModelNodeCtrl(SkinNode& skinNode_):
	Controller(CT_SKEL_ANIM_SKIN_NODE, skinNode_),
	frame(0.0),
	skinNode(skinNode_)
{}


//======================================================================================================================
// interpolate                                                                                                         =
//======================================================================================================================
void SkelAnimModelNodeCtrl::interpolate(const SkelAnim& animation, float frame,
                                        Vec<Vec3>& boneTranslations, Vec<Mat3>& boneRotations)
{
	ASSERT(frame < animation.getFramesNum());

	// calculate the t (used in slerp and lerp) using the keyframs and the frame and
	// calc the lPose and rPose witch indicate the pose IDs in witch the frame lies between
	const Vec<uint>& keyframes = animation.getKeyframes();
	float t = 0.0;
	uint lPose = 0, rPose = 0;
	for(uint j=0; j<keyframes.size(); j++)
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
			t = (frame - (float)keyframes[lPose]) / float(keyframes[rPose] - keyframes[lPose]);
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

		// if the bone has animations then slerp and lerp to find the rotation and translation
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


//======================================================================================================================
// updateBoneTransforms                                                                                                =
//======================================================================================================================
void SkelAnimModelNodeCtrl::updateBoneTransforms(const Skeleton& skeleton,
                                                 Vec<Vec3>& boneTranslations, Vec<Mat3>& boneRotations)
{
	uint queue[128];
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
		// where MA is bone matrix at armature space and ANIM the interpolated transformation.
		combineTransformations(boneTranslations[boneId], boneRotations[boneId],
		                       boned.getTslSkelSpaceInv(), boned.getRotSkelSpaceInv(),
		                       boneTranslations[boneId], boneRotations[boneId]);

		combineTransformations(boned.getTslSkelSpace(), boned.getRotSkelSpace(),
		                       boneTranslations[boneId], boneRotations[boneId],
		                       boneTranslations[boneId], boneRotations[boneId]);

		// and finaly add the parent's transform
		if(boned.getParent())
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			combineTransformations(boneTranslations[boned.getParent()->getPos()],
			                       boneRotations[boned.getParent()->getPos()],
			                       boneTranslations[boneId], boneRotations[boneId],
			                       boneTranslations[boneId], boneRotations[boneId]);
		}

		// now add the bone's childes
		for(uint i = 0; i < boned.getChildsNum(); i++)
		{
			queue[tail++] = boned.getChild(i).getPos();
		}
	}
}


//======================================================================================================================
// deform                                                                                                              =
//======================================================================================================================
void SkelAnimModelNodeCtrl::deform(const Skeleton& skeleton, const Vec<Vec3>& boneTranslations,
                                   const Vec<Mat3>& boneRotations, Vec<Vec3>& heads, Vec<Vec3>& tails)
{
	for(uint i = 0; i < skeleton.getBones().size(); i++)
	{
		const Mat3& rot = boneRotations[i];
		const Vec3& transl = boneTranslations[i];

		heads[i] = skeleton.getBones()[i].getHead().getTransformed(transl, rot);
		tails[i] = skeleton.getBones()[i].getTail().getTransformed(transl, rot);
	}
}


//======================================================================================================================
// update                                                                                                              =
//======================================================================================================================
void SkelAnimModelNodeCtrl::update(float)
{
	frame += step;
	if(frame > skelAnim->getFramesNum()) // if the crnt is finished then play the next or loop the crnt
	{
		frame = 0.0;
	}

	if(!controlledNode.isVisible())
	{
		return;
	}

	interpolate(*skelAnim, frame, skinNode.getBoneTranslations(), skinNode.getBoneRotations());
	updateBoneTransforms(skinNode.getSkin().getSkeleton(), skinNode.getBoneTranslations(), skinNode.getBoneRotations());

	if(MainRendererSingleton::getInstance().getDbg().isEnabled() &&
	   MainRendererSingleton::getInstance().getDbg().isShowSkeletonsEnabled())
	{
		deform(skinNode.getSkin().getSkeleton(), skinNode.getBoneTranslations(), skinNode.getBoneRotations(),
		       skinNode.getHeads(), skinNode.getTails());
	}

	BOOST_FOREACH(SkinPatchNode* skinPatchNode, skinNode.getPatcheNodes())
	{
		MainRendererSingleton::getInstance().getSkinsDeformer().deform(*skinPatchNode);
	}
}
