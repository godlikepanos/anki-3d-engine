#include "model.h"
#include "assets.h"
#include "material.h"


/**
=======================================================================================================================================
model's data                                                                                                                          =
=======================================================================================================================================
*/


/*
=======================================================================================================================================
skeleton_data_t::Load                                                                                                                 =
=======================================================================================================================================
*/
bool skeleton_data_t::Load( const char* filename )
{
	fstream file( filename, ios::in );
	char str[200];

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return false;
	}

	// bones num
	int bones_num;
	file >> str >> bones_num;
	bones.resize( bones_num );

	for( uint i=0; i<bones.size(); i++ )
	{
		bone_data_t& bone = bones[i];
		bone.id = i;
		// name
		file >> str >> str >> str >> str;
		bone.SetName( str );

		// head
		file >> str;
		for( int j=0; j<3; j++ )
			file >> bone.head[j];

		// tail
		file >> str;
		for( int j=0; j<3; j++ )
			file >> bone.tail[j];

		// matrix
		mat4_t m4;
		file >> str;
		for( int j=0; j<4; j++ )
			for( int k=0; k<4; k++ )
				file >> m4(j,k);

		// matrix for real
		bone.rot_skel_space = m4.GetRotationPart();
		bone.tsl_skel_space = m4.GetTranslationPart();
		mat4_t MAi( m4.Inverted() );
		bone.rot_skel_space_inv = MAi.GetRotationPart();
		bone.tsl_skel_space_inv = MAi.GetTranslationPart();

		// parent
		int parent_id;
		file >> str >> parent_id;
		if( parent_id != -1 )
			bone.parent = &bones[parent_id];
		else
			bone.parent = NULL;

		// childs
		uint childs_num;
		file >> str >> childs_num >> str;
		DEBUG_ERR( childs_num>MAX_CHILDS_PER_BONE );
		bone.childs_num = childs_num;
		for( int j=0; j<bone.childs_num; j++ )
		{
			int child_id;
			file >> child_id;
			bone.childs[j] = &bones[child_id];
		}

	}

	file.close();
	return true;
}


/*
=======================================================================================================================================
model_data_t::LoadVWeights                                                                                                            =
=======================================================================================================================================
*/
bool model_data_t::LoadVWeights( const char* filename )
{
	fstream file( filename, ios::in );
	char str[200];

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return false;
	}

	uint verts_num;
	file >> str >> verts_num;

	// check if all verts have weights. This pressent time we treat
	// as error if one or more verts dont have weigths.
	if( verts_num != mesh_data->verts.size() )
	{
		ERROR( "verts_num != mesh_data->verts.size()" );
		return false;
	}

	vert_weights.resize( verts_num );
	for( size_t i=0; i<vert_weights.size(); i++ )
	{
		uint bones_num;
		file >> str >> str >> str >> bones_num;

		// we treat as error if one vert doesnt have a bone
		if( bones_num < 1 )
		{
			ERROR( "Vert \"" << i << "\" doesnt have at least one bone" );
			file.close();
			return false;
		}

		// and here is another possible error
		if( bones_num > MAX_BONES_PER_VERT )
		{
			ERROR( "Cannot have more than " << MAX_BONES_PER_VERT << " bones per vertex" );
			file.close();
			return false;
		}

		vert_weights[i].bones_num = bones_num;
		for( uint j=0; j<bones_num; j++ )
		{
			int bone_id;
			float weight;
			file >> str >> bone_id >> str >> weight;
			vert_weights[i].bones[j] = &skeleton_data->bones[ bone_id ];
			vert_weights[i].weights[j] = weight;
		}
	}

	file.close();
	return true;
}


/*
=======================================================================================================================================
model_data_t::Load                                                                                                                    =
=======================================================================================================================================
*/
bool model_data_t::Load( const char* filename )
{
	fstream file( filename, ios::in );
	char str[256];

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return false;
	}

	// mesh
	file >> str >> str;
	mesh_data = ass::LoadMeshD( str );
	if( !mesh_data ) return false;

	// skeleton
	file >> str >> str;
	skeleton_data = new skeleton_data_t;
	if( !skeleton_data->Load( str ) ) return false;

	// vert weights
	file >> str >> str;
	if( !LoadVWeights( str ) ) return false;

	file.close();
	return true;
}



/**
=======================================================================================================================================
skeleton animation                                                                                                                    =
=======================================================================================================================================
*/


/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool skeleton_anim_t::Load( const char* filename )
{
	fstream file( filename, ios::in );
	char str[200];
	int tmpi;

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return false;
	}

	// read the keyframes
	file >> str >> tmpi >> str; // the keyframes num
	keyframes.resize( tmpi );

	for( uint i=0; i<keyframes.size(); i++ )
	{
		file >> tmpi;
		keyframes[i] = tmpi;
	}

	// frames_num
	frames_num = keyframes[ keyframes.size()-1 ] + 1;

	// bones
	file >> str >> tmpi;
	bones.resize( tmpi );

	for( uint i=0; i<bones.size(); i++ )
	{
		file >> str >> str >> str >> str >> str >> tmpi;
		if( tmpi ) // if has animation
		{
			bones[i].keyframes.resize( keyframes.size() );

			for( uint j=0; j<keyframes.size(); j++ )
			{
				file >> str >> str >> str;
				for( int k=0; k<4; k++ )
					file >> bones[i].keyframes[j].rotation[k];

				file >> str;
				for( int k=0; k<3; k++ )
					file >> bones[i].keyframes[j].translation[k];
			}
		}
	}

	file.close();
	return true;
}


/**
=======================================================================================================================================
model                                                                                                                                 =
=======================================================================================================================================
*/


/*
=======================================================================================================================================
model_t::Init                                                                                                                         =
=======================================================================================================================================
*/
void model_t::Init( model_data_t* model_data_ )
{
	model_data = model_data_;

	// init the bones
	bones.resize( model_data->skeleton_data->bones.size() );

	// init the verts
	verts.resize( model_data->mesh_data->verts.size() );
}


/*
=======================================================================================================================================
model_t::Interpolate                                                                                                                  =
=======================================================================================================================================
*/
void model_t::Interpolate( skeleton_anim_t* anim, float frame )
{
	DEBUG_ERR( frame >= anim->frames_num );

	// calculate the t (used in slerp and lerp) and
	// calc the l_pose and r_pose witch indicate the pose ids in witch the frame lies between
	const vector<uint>& keyframes = anim->keyframes;
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
	for( uint bone_id=0; bone_id<bones.size(); bone_id++ )
	{
		const bone_anim_t& banim = anim->bones[bone_id];

		mat3_t& local_rot = bones[bone_id].rotation;
		vec3_t& local_transl = bones[bone_id].translation;

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if( banim.keyframes.size() != 0 )
		{
			const bone_pose_t& l_bpose = banim.keyframes[l_pose];
			const bone_pose_t& r_bpose = banim.keyframes[r_pose];

			// rotation
			const quat_t& q0 = l_bpose.rotation;
			const quat_t& q1 = r_bpose.rotation;
			local_rot.Set( q0.Slerp(q1, t) );

			// translation
			const vec3_t& v0 = l_bpose.translation;
			const vec3_t& v1 = r_bpose.translation;
			local_transl = v0.Lerp( v1, t );
		}
		// else put the idents
		else
		{
			local_rot.SetIdent();
			local_transl.SetZero();
		}
	}

}


/*
=======================================================================================================================================
model_t::Interpolate                                                                                                                  =
=======================================================================================================================================
*/
void model_t::Interpolate()
{
	DEBUG_ERR( !model_data );

	for( ushort i=0; i<MAX_SIMULTANEOUS_ANIMS; i++ )
	{
		action_t* crnt = &crnt_actions[i];
		action_t* next = &next_actions[i];

		if( crnt->anim == NULL ) continue; // if the slot doesnt have anim dont bother

		if( crnt->frame + crnt->step > crnt->anim->frames_num ) // if the crnt is finished then play the next or loop the crnt
		{
			if( next->anim == NULL ) // if there is no next anim then loop the crnt
				crnt->frame = 0.0;
			else // else play the next
			{
				crnt->anim = next->anim;
				crnt->step = next->step;
				crnt->frame = 0.0;
				next->anim = NULL;
			}
		}

		Interpolate( crnt->anim, crnt->frame );

		crnt->frame += crnt->step; // inc the frame
	}

	UpdateBoneTransforms();
}


/*
=======================================================================================================================================
model_t::UpdateBoneTransforms                                                                                                         =
=======================================================================================================================================
*/
void model_t::UpdateBoneTransforms()
{
	uint queue[ 128 ];
	uint head = 0, tail = 0;


	// put the roots
	for( uint i=0; i<bones.size(); i++ )
		if( model_data->skeleton_data->bones[i].parent == NULL )
			queue[tail++] = i; // queue push

	// loop
	while( head != tail ) // while queue not empty
	{
		uint bone_id = queue[head++]; // queue pop
		const bone_data_t& boned = model_data->skeleton_data->bones[bone_id];
		bone_t& bone = bones[bone_id];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated transformation.
		CombineTransformations( bone.translation, bone.rotation,
		                        boned.tsl_skel_space_inv, boned.rot_skel_space_inv,
		                        bone.translation, bone.rotation );

		CombineTransformations( boned.tsl_skel_space, boned.rot_skel_space,
		                        bone.translation, bone.rotation,
		                        bone.translation, bone.rotation );

		// and finaly add the parent's transform
		if( boned.parent )
		{
			const bone_t& parent = bones[ boned.parent->id ];
			// bone.final_final_transform = parent.transf * bone.final_transform
			CombineTransformations( parent.translation, parent.rotation,
		                          bone.translation, bone.rotation,
		                          bone.translation, bone.rotation );
		}

		// now add the bone's childes
		for( uint i=0; i<boned.childs_num; i++ )
			queue[tail++] = boned.childs[i]->id;
	}
}


/*
=======================================================================================================================================
model_t::Play                                                                                                                         =
=======================================================================================================================================
*/
void model_t::Play( skeleton_anim_t* anim, ushort slot, float step, ushort play_type, float smooth_transition_frames )
{
	DEBUG_ERR( !model_data );
	DEBUG_ERR( anim->bones.size() != model_data->skeleton_data->bones.size() ); // the anim is not for this skeleton
	DEBUG_ERR( slot >= MAX_SIMULTANEOUS_ANIMS ); // wrong slot

	switch( play_type )
	{
		case START_IMMEDIATELY:
			crnt_actions[slot].anim = anim;
			crnt_actions[slot].step = step;
			crnt_actions[slot].frame = 0.0;
			break;

		case WAIT_CRNT_TO_FINISH:
			next_actions[slot].anim = anim;
			next_actions[slot].step = step;
			next_actions[slot].frame = 0.0;
			break;

		case SMOOTH_TRANSITION:
			DEBUG_ERR( true ); // unimplemented
			break;

		default:
			DEBUG_ERR( true );
	}
}


/*
=======================================================================================================================================
model_t::Deform                                                                                                                       =
=======================================================================================================================================
*/
void model_t::Deform()
{
	DEBUG_ERR( !model_data );
	DEBUG_ERR( verts.size() == 0 );


	// deform the verts
	for( uint i=0; i<model_data->mesh_data->verts.size(); i++ ) // for all verts
	{
		const vertex_weight_t& vw = model_data->vert_weights[i];

		// a small optimazation detour in case the vert has only one bone
		if( vw.bones_num == 1 )
		{
			uint bid = vw.bones[0]->id;
			const mat3_t& rot = bones[ bid ].rotation;
			const vec3_t& transl = bones[ bid ].translation;

			verts[i].coords = model_data->mesh_data->verts[i].coords.Transformed( transl, rot );
			verts[i].normal = rot * model_data->mesh_data->verts[i].normal;
			continue;
		}

		// calc the matrix according the weights
		mat3_t m3;
		m3.SetZero();
		vec3_t v3;
		v3.SetZero();
		// for all bones of this vert
		for( int j=0; j< vw.bones_num; j++ )
		{
			uint bid = vw.bones[j]->id;

			m3 += bones[ bid ].rotation * vw.weights[j];
			v3 += bones[ bid ].translation * vw.weights[j];
		}

		// apply the matrix to the verts
		verts[i].coords = model_data->mesh_data->verts[i].coords.Transformed( v3, m3 );
		verts[i].normal = m3 * model_data->mesh_data->verts[i].normal;
	}

	// deform the heads and tails
	if( r::show_skeletons )
	{
		for( uint i=0; i<model_data->skeleton_data->bones.size(); i++ )
		{
			const mat3_t& rot = bones[ i ].rotation;
			const vec3_t& transl = bones[ i ].translation;

			bones[i].head = model_data->skeleton_data->bones[i].head.Transformed( transl, rot );
			bones[i].tail = model_data->skeleton_data->bones[i].tail.Transformed( transl, rot );
		}
	}

}


/*
=======================================================================================================================================
model_t::Render                                                                                                                       =
=======================================================================================================================================
*/
void model_t::Render()
{
	DEBUG_ERR( !model_data );

	// transform
	glPushMatrix();
	r::MultMatrix( world_transformation );

	// material
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_LIGHTING );
	glPolygonMode( GL_FRONT, GL_FILL );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	material_t* mat = ass::SearchMat( "imp.mat" );
	if( mat == NULL )
	{
		ERROR( "skata" );
	}
	else
		mat->Use();

	/*glEnable( GL_BLEND );
	glColor4fv( &vec4_t(1., 1., 1.0, 1.0)[0] );
	glBlendFunc( GL_ONE_MINUS_SRC_ALPHA, GL_ONE );*/
	//glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
	//glBlendFunc( GL_ONE, GL_ONE_MINUS_DST_ALPHA );

	// draw
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 3, GL_FLOAT, sizeof(vertex_t), &verts[0].coords[0] );
	glNormalPointer( GL_FLOAT, sizeof(vertex_t), &verts[0].normal[0] );
	glTexCoordPointer( 2, GL_FLOAT, 0, &model_data->mesh_data->uvs[0] );

	glDrawElements( GL_TRIANGLES, model_data->mesh_data->vert_list.size(), GL_UNSIGNED_INT, &model_data->mesh_data->vert_list[0] );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	if( r::show_axis )
	{
		RenderAxis();
	}


	// reposition and draw the skeleton
	if( r::show_skeletons )
	{

		glDisable( GL_DEPTH_TEST );
		for( uint i=0; i<model_data->skeleton_data->bones.size(); i++ )
		{

			glPointSize( 4.0f );
			glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
			glBegin( GL_POINTS );
				glVertex3fv( &bones[i].head[0] );
			glEnd();

			glBegin( GL_LINES );
				glVertex3fv( &bones[i].head[0] );
				glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
				glVertex3fv( &bones[i].tail[0] );
			glEnd();
		}
	}

	// end
	glPopMatrix();
}



























