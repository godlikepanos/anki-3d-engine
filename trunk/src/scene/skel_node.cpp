#include "skel_node.h"
#include "renderer.h"
#include "skel_anim.h"
#include "skeleton.h"


//=====================================================================================================================================
// Interpolate                                                                                                                        =
//=====================================================================================================================================
void skel_anim_controller_t::Interpolate( skel_anim_t* animation, float frame )
{
	DEBUG_ERR( frame >= skel_anim->frames_num );

	// calculate the t (used in slerp and lerp) and
	// calc the l_pose and r_pose witch indicate the pose ids in witch the frame lies between
	const vec_t<uint>& keyframes = skel_anim->keyframes;
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
	for( uint i=0; i<bone_rotations.size(); i++ )
	{
		const skel_anim_t::bone_anim_t& banim = skel_anim->bones[i];

		mat3_t& local_rot = bone_rotations[i];
		vec3_t& local_transl = bone_translations[i];

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if( banim.keyframes.size() != 0 )
		{
			const skel_anim_t::bone_pose_t& l_bpose = banim.keyframes[l_pose];
			const skel_anim_t::bone_pose_t& r_bpose = banim.keyframes[r_pose];

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
void skel_anim_controller_t::UpdateBoneTransforms()
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
		const skeleton_t::bone_t& boned = skel_node->skeleton->bones[bone_id];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated transformation.
		CombineTransformations( bone_translations[bone_id], bone_rotations[bone_id],
		                        boned.tsl_skel_space_inv, boned.rot_skel_space_inv,
		                        bone_translations[bone_id], bone_rotations[bone_id] );

		CombineTransformations( boned.tsl_skel_space, boned.rot_skel_space,
		                        bone_translations[bone_id], bone_rotations[bone_id],
		                        bone_translations[bone_id], bone_rotations[bone_id] );

		// and finaly add the parent's transform
		if( boned.parent )
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			CombineTransformations( bone_translations[boned.parent->id], bone_rotations[boned.parent->id],
		                          bone_translations[bone_id], bone_rotations[bone_id],
		                          bone_translations[bone_id], bone_rotations[bone_id] );
		}

		// now add the bone's childes
		for( uint i=0; i<boned.childs_num; i++ )
			queue[tail++] = boned.childs[i]->id;
	}
}


//=====================================================================================================================================
// Deform                                                                                                                             =
//=====================================================================================================================================
void skel_anim_controller_t::Deform()
{
	skeleton_t* skeleton = skel_node->skeleton;

	for( uint i=0; i<skeleton->bones.size(); i++ )
	{
		const mat3_t& rot = bone_rotations[ i ];
		const vec3_t& transl = bone_translations[ i ];

		heads[i] = skeleton->bones[i].head.GetTransformed( transl, rot );
		tails[i] = skeleton->bones[i].tail.GetTransformed( transl, rot );
	}
}


//=====================================================================================================================================
// Update                                                                                                                             =
//=====================================================================================================================================
void skel_anim_controller_t::Update()
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


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void skel_node_t::Init( const char* filename )
{
	skeleton = rsrc::skeletons.Load( filename );
}

//=====================================================================================================================================
// Deinit                                                                                                                             =
//=====================================================================================================================================
void skel_node_t::Deinit()
{
	rsrc::skeletons.Unload( skeleton );
}


//=====================================================================================================================================
// Render                                                                                                                             =
//=====================================================================================================================================
void skel_node_t::Render()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	//glPointSize( 4.0f );
	//glLineWidth( 2.0f );

	for( uint i=0; i<skeleton->bones.size(); i++ )
	{
		glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
		glBegin( GL_POINTS );
			glVertex3fv( &skel_anim_controller->heads[0][0] );
		glEnd();

		glBegin( GL_LINES );
			glVertex3fv( &skel_anim_controller->heads[0][0] );
			glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
			glVertex3fv( &skel_anim_controller->tails[0][0] );
		glEnd();
	}

	glPopMatrix();
}
