#include "model.h"


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
	bones.Malloc( bones_num );

	for( uint i=0; i<bones.Size(); i++ )
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
		int childs_num;
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
	if( verts_num != verts.Size() )
	{
		ERROR( "verts_num != verts.Size()" );
		return false;
	}

	vert_weights.Malloc( verts_num );
	for( size_t i=0; i<vert_weights.Size(); i++ )
	{
		int bones_num;
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
		for( int j=0; j<bones_num; j++ )
		{
			int bone_id;
			float weight;
			file >> str >> bone_id >> str >> weight;
			vert_weights[i].bones[j] = &bones[ bone_id ];
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
bool model_data_t::Load( const char* path )
{
	char filename [200];

	// get name
	strcpy( filename, path );
	filename[ strlen(path)-1 ] = '\0';
	SetName( CutPath( filename ) );

	// the mesh
	strcpy( filename, path );
	strcat( filename, "mesh.txt" );
	if( !mesh_data_t::Load( filename ) ) return false;

	// the skeleton
	strcpy( filename, path );
	strcat( filename, "skeleton.txt" );
	if( !skeleton_data_t::Load( filename ) ) return false;


	// vert weights
	strcpy( filename, path );
	strcat( filename, "vertweights.txt" );
	if( !LoadVWeights( filename ) ) return false;

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

	// read the name
	file >> str >> str;
	SetName(str);

	// read the keyframes
	file >> str >> tmpi >> str; // the keyframes num
	keyframes.Malloc( tmpi );

	for( uint i=0; i<keyframes.Size(); i++ )
	{
		file >> tmpi;
		keyframes[i] = tmpi;
	}

	// frames_num
	frames_num = keyframes[ keyframes.Size()-1 ];

	// bones
	file >> str >> tmpi;
	bones.Malloc( tmpi );

	for( uint i=0; i<bones.Size(); i++ )
	{
		file >> str >> str >> str >> str >> str >> tmpi;
		if( tmpi ) // if has animation
		{
			bones[i].keyframes.Malloc( keyframes.Size() );

			for( uint j=0; j<keyframes.Size(); j++ )
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
	SetName( model_data->GetName() );

	// init the bones
	bones.Malloc( model_data->bones.Size() );

	for( uint i=0; i<model_data->bones.Size(); i++ )
	{
		bone_data_t* bonedata = &model_data->bones[i];
		bone_t* my_bone = &bones[i];

		// set the parent
		if( bonedata->parent != NULL )
			my_bone->parent = &bones[ bonedata->parent->id ];

		// set the childs
		if( bonedata->childs_num != 0 )
		{
			my_bone->childs.Malloc( bonedata->childs_num );

			for( uint j=0; j<bonedata->childs_num; j++ )
				my_bone->childs[j] = &bones[ bonedata->childs[j]->id ];
		}
	}
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
	const array_t<uint>& keyframes = anim->keyframes;
	float t = 0.0;
	uint l_pose = 0, r_pose = 0;
	for( uint j=0; j<keyframes.Size(); j++ )
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
	for( uint bone_id=0; bone_id<bone_poses.Size(); bone_id++ )
	{
		const bone_anim_t& banim = anim->bones[bone_id];

		quat_t& local_rot = bone_poses[bone_id].rotation;
		vec3_t& local_transl = bone_poses[bone_id].translation;

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if( banim.keyframes.Size() != 0 )
		{
			const bone_pose_t& l_bpose = banim.keyframes[l_pose];
			const bone_pose_t& r_bpose = banim.keyframes[r_pose];

			// rotation
			const quat_t& q0 = l_bpose.rotation;
			const quat_t& q1 = r_bpose.rotation;
			local_rot = q0.Slerp(q1, t);

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
	DEBUG_ERR( bone_poses.Size() != 0 ); // the array must be empty

	bone_poses.Malloc( model_data->bones.Size() );

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
}


/*
=======================================================================================================================================
model_t::Play                                                                                                                         =
=======================================================================================================================================
*/
void model_t::Play( skeleton_anim_t* anim, ushort slot, float step, ushort play_type, float smooth_transition_frames )
{
	DEBUG_ERR( !model_data );
	DEBUG_ERR( anim->bones.Size() != model_data->bones.Size() );
	DEBUG_ERR( slot >= MAX_SIMULTANEOUS_ANIMS );

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
model_t::UpdateTransformations                                                                                                        =
=======================================================================================================================================
*/
void model_t::UpdateTransformations()
{
	DEBUG_ERR( !model_data || bone_poses.Size()==0 || rotations.Size()!=0 || translations.Size()!=0 );

	// allocate
	rotations.Malloc( model_data->bones.Size() );
	translations.Malloc( model_data->bones.Size() );


	// using the in depth search algorithm animate the bones
	// from father to child cause the child needs tha father's transformation matrix
	ushort temp[200];
	queue_t<ushort> queue( temp, 200 ); // the queue for the seatch. Takes bone ids

	array_t<bone_data_t>& bones = model_data->bones;
	// put the roots (AKA the bones without father) in the queue
	for( uint i=0; i<bones.Size(); i++ )
	{
		if( bones[i].parent == NULL )
			queue.Push( i );
	}

	// while queue is not empty
	while( !queue.IsEmpty() )
	{
		// queue stuff
		int bone_id = queue.Pop();
		const bone_data_t& bone = bones[bone_id];

		/// start the calculations

		// m4 = MA * ANIM * MAi
		// where  MA is bone matrix at armature space and ANIM the interpolated transformation.
		CombineTransformations( bone_poses[bone_id].translation, bone_poses[bone_id].rotation,
		                        bone.tsl_skel_space_inv, bone.rot_skel_space_inv,
		                        translations[bone_id], rotations[bone_id] );

		CombineTransformations( bone.tsl_skel_space, bone.rot_skel_space,
		                        translations[bone_id], rotations[bone_id],
		                        translations[bone_id], rotations[bone_id] );

		// 1: Apply the parent's transformation
		if( bone.parent )
		{
			// AKA transf_final = parent.transf * mine
			CombineTransformations( translations[bone.parent->id], rotations[bone.parent->id],
		                          translations[bone_id], rotations[bone_id],
		                          translations[bone_id], rotations[bone_id] );
		}

		/// end of calculations

		// queue stuff
		for( int i=0; i<bone.childs_num; i++ )
			queue.Push( bone.childs[i]->id );
	}

	// deallocate
	bone_poses.Free();
}


/*
=======================================================================================================================================
model_t::Deform                                                                                                                       =
=======================================================================================================================================
*/
void model_t::Deform()
{
	DEBUG_ERR( !model_data || rotations.Size()==0 || translations.Size()==0 || verts.Size()!=0 );

	// allocate
	verts.Malloc( model_data->verts.Size() );

	// deform the verts
	for( uint i=0; i<model_data->verts.Size(); i++ ) // for all verts
	{
		const vertex_weight_t& vw = model_data->vert_weights[i];

		// a small optimazation detour
		if( vw.bones_num == 1 )
		{
			verts[i].coords = model_data->verts[i].coords.Transformed( translations[ vw.bones[0]->id ], rotations[ vw.bones[0]->id ] );
			verts[i].normal = rotations[ vw.bones[0]->id ] * model_data->verts[i].normal;
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
			m3 += rotations[ vw.bones[j]->id ] * vw.weights[j];
			v3 += translations[ vw.bones[j]->id ] * vw.weights[j];
		}

		// apply the matrix to the verts
		verts[i].coords = model_data->verts[i].coords.Transformed( v3, m3 );
		verts[i].normal = m3 * model_data->verts[i].normal;
	}

	// deform the heads and tails
	if( r::show_skeletons )
	{
		heads.Malloc( model_data->bones.Size() );
		tails.Malloc( model_data->bones.Size() );

		for( uint i=0; i<model_data->bones.Size(); i++ )
		{
			heads[i] = model_data->bones[i].head.Transformed( translations[i], rotations[i] );
			tails[i] = model_data->bones[i].tail.Transformed( translations[i], rotations[i] );
		}
	}

	// deallocate
	rotations.Free();
	translations.Free();
}


/*
=======================================================================================================================================
model_t::Render                                                                                                                       =
=======================================================================================================================================
*/
void model_t::Render()
{
	DEBUG_ERR( !model_data || verts.Size()==0 );

	// transform
	glPushMatrix();
	r::MultMatrix( world_transform );

	// material
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_LIGHTING );
	glPolygonMode( GL_FRONT, GL_FILL );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	// draw
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 3, GL_FLOAT, sizeof(vertex_t), &verts[0].coords[0] );
	glNormalPointer( GL_FLOAT, sizeof(vertex_t), &verts[0].normal[0] );
	glTexCoordPointer( 2, GL_FLOAT, 0, &model_data->uvs[0] );

	glDrawElements( GL_TRIANGLES, model_data->vert_list.Size(), GL_UNSIGNED_INT, &model_data->vert_list[0] );

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
		DEBUG_ERR( heads.Size() != model_data->bones.Size() || tails.Size() != model_data->bones.Size() );

		glDisable( GL_DEPTH_TEST );
		for( uint i=0; i<model_data->bones.Size(); i++ )
		{

			glPointSize( 4.0f );
			glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
			glBegin( GL_POINTS );
				glVertex3fv( &heads[i][0] );
			glEnd();

			glBegin( GL_LINES );
				glVertex3fv( &heads[i][0] );
				glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
				glVertex3fv( &tails[i][0] );
			glEnd();
		}

		heads.Free();
		tails.Free();
	}

	// end
	glPopMatrix();

	// deallocate
	verts.Free();
}



























