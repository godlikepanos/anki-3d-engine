#include "SkelAnimCtrl.h"
#include "SkelAnim.h"
#include "SkelNode.h"
#include "Skeleton.h"
#include "App.h"
#include "MainRenderer.h"


//=====================================================================================================================================
// SkelAnimCtrl                                                                                                             =
//=====================================================================================================================================
SkelAnimCtrl::SkelAnimCtrl( SkelNode* skelNode_ ):
	Controller(CT_SKEL_ANIM),
	skelNode( skelNode_ )
{
	heads.resize( skelNode->skeleton->bones.size() );
	tails.resize( skelNode->skeleton->bones.size() );
	boneRotations.resize( skelNode->skeleton->bones.size() );
	boneTranslations.resize( skelNode->skeleton->bones.size() );
}


//=====================================================================================================================================
// interpolate                                                                                                                        =
//=====================================================================================================================================
void SkelAnimCtrl::interpolate( SkelAnim* animation, float frame )
{
	DEBUG_ERR( frame >= animation->framesNum );

	// calculate the t (used in slerp and lerp) using the keyframs and the frame and
	// calc the lPose and rPose witch indicate the pose ids in witch the frame lies between
	const Vec<uint>& keyframes = animation->keyframes;
	float t = 0.0;
	uint lPose = 0, rPose = 0;
	for( uint j=0; j<keyframes.size(); j++ )
	{
		if( (float)keyframes[j] == frame )
		{
			lPose = rPose = j;
			t = 0.0;
			break;
		}
		else if( (float)keyframes[j] > frame )
		{
			lPose = j-1;
			rPose = j;
			t = ( frame - (float)keyframes[lPose] ) / float( keyframes[rPose] - keyframes[lPose] );
			break;
		}
	}


	// now for all bones update bone's poses
	DEBUG_ERR( boneRotations.size()<1 );
	for( uint i=0; i<boneRotations.size(); i++ )
	{
		const SkelAnim::BoneAnim& banim = animation->bones[i];

		Mat3& localRot = boneRotations[i];
		Vec3& localTransl = boneTranslations[i];

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if( banim.keyframes.size() != 0 )
		{
			const SkelAnim::BonePose& lBpose = banim.keyframes[lPose];
			const SkelAnim::BonePose& rBpose = banim.keyframes[rPose];

			// rotation
			const Quat& q0 = lBpose.rotation;
			const Quat& q1 = rBpose.rotation;
			localRot = Mat3( q0.slerp(q1, t) );

			// translation
			const Vec3& v0 = lBpose.translation;
			const Vec3& v1 = rBpose.translation;
			localTransl = v0.lerp( v1, t );
		}
		// else put the idents
		else
		{
			localRot = Mat3::getIdentity();
			localTransl = Vec3( 0.0, 0.0, 0.0 );
		}
	}
}


//=====================================================================================================================================
// updateBoneTransforms                                                                                                               =
//=====================================================================================================================================
void SkelAnimCtrl::updateBoneTransforms()
{
	uint queue[ 128 ];
	uint head = 0, tail = 0;

	// put the roots
	for( uint i=0; i<skelNode->skeleton->bones.size(); i++ )
		if( skelNode->skeleton->bones[i].parent == NULL )
			queue[tail++] = i; // queue push

	// loop
	while( head != tail ) // while queue not empty
	{
		uint boneId = queue[head++]; // queue pop
		const Skeleton::Bone& boned = skelNode->skeleton->bones[boneId];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated transformation.
		combineTransformations( boneTranslations[boneId], boneRotations[boneId],
		                        boned.tslSkelSpaceInv, boned.rotSkelSpaceInv,
		                        boneTranslations[boneId], boneRotations[boneId] );

		combineTransformations( boned.tslSkelSpace, boned.rotSkelSpace,
		                        boneTranslations[boneId], boneRotations[boneId],
		                        boneTranslations[boneId], boneRotations[boneId] );

		// and finaly add the parent's transform
		if( boned.parent )
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			combineTransformations( boneTranslations[boned.parent->id], boneRotations[boned.parent->id],
		                          boneTranslations[boneId], boneRotations[boneId],
		                          boneTranslations[boneId], boneRotations[boneId] );
		}

		// now add the bone's childes
		for( uint i=0; i<boned.childsNum; i++ )
			queue[tail++] = boned.childs[i]->id;
	}
}


//=====================================================================================================================================
// deform                                                                                                                             =
//=====================================================================================================================================
void SkelAnimCtrl::deform()
{
	Skeleton* skeleton = skelNode->skeleton;

	for( uint i=0; i<skeleton->bones.size(); i++ )
	{
		const Mat3& rot = boneRotations[ i ];
		const Vec3& transl = boneTranslations[ i ];

		heads[i] = skeleton->bones[i].head.getTransformed( transl, rot );
		tails[i] = skeleton->bones[i].tail.getTransformed( transl, rot );
	}
}


//=====================================================================================================================================
// update                                                                                                                             =
//=====================================================================================================================================
void SkelAnimCtrl::update( float )
{
	frame += step;
	if( frame > skelAnim->framesNum ) // if the crnt is finished then play the next or loop the crnt
	{
		frame = 0.0;
	}

	interpolate( skelAnim, frame );
	updateBoneTransforms();
	if( app->getMainRenderer()->dbg.isShowSkeletonsEnabled() )
	{
		deform();
	}
}
