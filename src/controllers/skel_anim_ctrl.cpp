#include "skel_anim_ctrl.h"
#include "SkelAnim.h"
#include "skel_node.h"
#include "Skeleton.h"
#include "renderer.h"


//=====================================================================================================================================
// skel_anim_ctrl_t                                                                                                             =
//=====================================================================================================================================
skel_anim_ctrl_t::skel_anim_ctrl_t( skel_node_t* skel_node_ ):
	controller_t(CT_SKEL_ANIM),
	skel_node( skel_node_ )
{
	heads.resize( skel_node->skeleton->bones.size() );
	tails.resize( skel_node->skeleton->bones.size() );
	bone_rotations.resize( skel_node->skeleton->bones.size() );
	Boneranslations.resize( skel_node->skeleton->bones.size() );
}


//=====================================================================================================================================
// Interpolate                                                                                                                        =
//=====================================================================================================================================
void skel_anim_ctrl_t::Interpolate( SkelAnim* animation, float frame )
{
	DEBUG_ERR( frame >= animation->frames_num );

	// calculate the t (used in slerp and lerp) and
	// calc the l_pose and r_pose witch indicate the pose ids in witch the frame lies between
	const Vec<uint>& keyframes = animation->keyframes;
	float t = 0.0;
	uint l_pose = 0, r_pose = 0;
	for( uint j=0; j<keyframes.size(); j++ )
	{
		if( (float)keyframes[j] == frame )
		{
			l_pose = r_pose = j;
			t = 0.0;
			break;
		}
		else if( (float)keyframes[j] > frame )
		{
			l_pose = j-1;
			r_pose = j;
			t = ( frame - (float)keyframes[l_pose] ) / float( keyframes[r_pose] - keyframes[l_pose] );
			break;
		}
	}


	// now for all bones update bone's poses
	DEBUG_ERR( bone_rotations.size()<1 );
	for( uint i=0; i<bone_rotations.size(); i++ )
	{
		const SkelAnim::BoneAnim& banim = animation->bones[i];

		mat3_t& local_rot = bone_rotations[i];
		vec3_t& local_transl = Boneranslations[i];

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if( banim.keyframes.size() != 0 )
		{
			const SkelAnim::BonePose& l_bpose = banim.keyframes[l_pose];
			const SkelAnim::BonePose& r_bpose = banim.keyframes[r_pose];

			// rotation
			const quat_t& q0 = l_bpose.rotation;
			const quat_t& q1 = r_bpose.rotation;
			local_rot = mat3_t( q0.Slerp(q1, t) );

			// translation
			const vec3_t& v0 = l_bpose.translation;
			const vec3_t& v1 = r_bpose.translation;
			local_transl = v0.Lerp( v1, t );
		}
		// else put the idents
		else
		{
			local_rot = mat3_t::GetIdentity();
			local_transl = vec3_t( 0.0, 0.0, 0.0 );
		}
	}
}


//=====================================================================================================================================
// UpdateBoneTransforms                                                                                                               =
//=====================================================================================================================================
void skel_anim_ctrl_t::UpdateBoneTransforms()
{
	uint queue[ 128 ];
	uint head = 0, tail = 0;

	// put the roots
	for( uint i=0; i<skel_node->skeleton->bones.size(); i++ )
		if( skel_node->skeleton->bones[i].parent == NULL )
			queue[tail++] = i; // queue push

	// loop
	while( head != tail ) // while queue not empty
	{
		uint bone_id = queue[head++]; // queue pop
		const Skeleton::Bone& boned = skel_node->skeleton->bones[bone_id];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated transformation.
		CombineTransformations( Boneranslations[bone_id], bone_rotations[bone_id],
		                        boned.tslSkelSpaceInv, boned.rotSkelSpaceInv,
		                        Boneranslations[bone_id], bone_rotations[bone_id] );

		CombineTransformations( boned.tslSkelSpace, boned.rotSkelSpace,
		                        Boneranslations[bone_id], bone_rotations[bone_id],
		                        Boneranslations[bone_id], bone_rotations[bone_id] );

		// and finaly add the parent's transform
		if( boned.parent )
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			CombineTransformations( Boneranslations[boned.parent->id], bone_rotations[boned.parent->id],
		                          Boneranslations[bone_id], bone_rotations[bone_id],
		                          Boneranslations[bone_id], bone_rotations[bone_id] );
		}

		// now add the bone's childes
		for( uint i=0; i<boned.childsNum; i++ )
			queue[tail++] = boned.childs[i]->id;
	}
}


//=====================================================================================================================================
// Deform                                                                                                                             =
//=====================================================================================================================================
void skel_anim_ctrl_t::Deform()
{
	Skeleton* skeleton = skel_node->skeleton;

	for( uint i=0; i<skeleton->bones.size(); i++ )
	{
		const mat3_t& rot = bone_rotations[ i ];
		const vec3_t& transl = Boneranslations[ i ];

		heads[i] = skeleton->bones[i].head.GetTransformed( transl, rot );
		tails[i] = skeleton->bones[i].tail.GetTransformed( transl, rot );
	}
}


//=====================================================================================================================================
// Update                                                                                                                             =
//=====================================================================================================================================
void skel_anim_ctrl_t::Update( float )
{
	frame += step;
	if( frame > skel_anim->frames_num ) // if the crnt is finished then play the next or loop the crnt
	{
		frame = 0.0;
	}

	Interpolate( skel_anim, frame );
	UpdateBoneTransforms();
	if( r::dbg::show_skeletons )
	{
		Deform();
	}
}
