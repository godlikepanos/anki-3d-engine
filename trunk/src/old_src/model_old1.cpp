#include "model.h"


/**
=======================================================================================================================================
model's data                                                                                                                          =
=======================================================================================================================================
*/


/*
=======================================================================================================================================
skeleton_t::Load                                                                                                                      =
=======================================================================================================================================
*/
bool skeleton_t::Load( char* filename )
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

	for( unsigned int i=0; i<bones.Size(); i++ )
	{
		bone_t& bone = bones[i];
		bone.id = i;
		// name
		file >> str >> str >> str >> &bone.name[0];

		// length
		file >> str >> bone.length;

		// head
		file >> str;
		for( int j=0; j<3; j++ )
			file >> bone.head_bs[j];

		file >> str;
		for( int j=0; j<3; j++ )
			file >> bone.head_as[j];

		// tail
		file >> str;
		for( int j=0; j<3; j++ )
			file >> bone.tail_bs[j];

		file >> str;
		for( int j=0; j<3; j++ )
			file >> bone.tail_as[j];

		// matrices
		file >> str;
		for( int j=0; j<3; j++ )
			for( int k=0; k<3; k++ )
				file >> bone.matrix_bs(j,k);

		file >> str;
		for( int j=0; j<4; j++ )
			for( int k=0; k<4; k++ )
				file >> bone.matrix_as(j,k);

		// matrix for real
		bone.rot_skel_space = bone.matrix_as.GetRotationPart();
		bone.tsl_skel_space = bone.matrix_as.GetTranslationPart();
		mat4_t MAi( bone.matrix_as.Inverted() );
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
bool model_data_t::LoadVWeights( char* filename )
{
	fstream file( filename, ios::in );
	char str[200];

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return false;
	}

	unsigned int verts_num;
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
bool model_data_t::Load( char* path )
{
	char filename [200];

	// the mesh
	strcpy( filename, path );
	strcat( filename, "mesh.txt" );
	if( !mesh_data_t::Load( filename ) ) return false;

	// the skeleton
	strcpy( filename, path );
	strcat( filename, "skeleton.txt" );
	if( !skeleton_t::Load( filename ) ) return false;


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
bool skeleton_anim_t::Load( char* filename )
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
	keyframes.Malloc( tmpi );

	for( unsigned int i=0; i<keyframes.Size(); i++ )
	{
		file >> tmpi;
		keyframes[i] = tmpi;
	}

	// bones
	file >> str >> tmpi;
	bones.Malloc( tmpi );

	for( unsigned int i=0; i<bones.Size(); i++ )
	{
		file >> str >> str >> str >> str >> str >> tmpi;
		if( tmpi ) // if has animation
		{
			bones[i].keyframes.Malloc( keyframes.Size() );

			for( unsigned int j=0; j<keyframes.Size(); j++ )
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

	verts.Malloc( model_data->verts.Size() );
	bones_state.Malloc( model_data->bones.Size() );
}


/*
=======================================================================================================================================
model_t::Interpolate                                                                                                                  =
=======================================================================================================================================
*/
void model_t::Interpolate( const skeleton_anim_t& anim, float frame )
{
	DEBUG_ERR( !model_data || anim.bones.Size()!=model_data->bones.Size() );

	// calculate the t (used in slerp and lerp) and
	// calc the l_pose and r_pose witch indicate the pose ids in witch the frame lies between
	const array_t<unsigned int>& keyframes = anim.keyframes;
	float t=0.0;
	int l_pose=0, r_pose=0;
	for( unsigned int j=0; j<keyframes.Size(); j++ )
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


	// using the in depth search algorithm animate the bones
	// from father to child cause the child needs tha father's transformation matrix
	unsigned short queue [200]; // the queue for the seatch. Takes bone ids
	int head = 0, tail = 0; // head and tail of queue


	array_t<bone_t>& bones = model_data->bones;
	// put the roots (AKA the bones without father) in the queue
	for( unsigned int i=0; i<bones.Size(); i++ )
	{
		if( bones[i].parent == NULL )
			queue[tail++] = i;
	}

	// while queue is not empty
	while( head!=tail )
	{
		int bone_id = queue[ head ];
		const bone_t& bone = bones[bone_id];
		const mat4_t& MA = bone.matrix_as;

		/// now we have the bone. Do the calculations

		// 0: calculate the interpolated rotation and translation
		quat_t& local_rot = bones_state[bone_id].rotation;
		vec3_t& local_transl = bones_state[bone_id].translation;
		const bone_anim_t& banim = anim.bones[bone_id];
		if( banim.keyframes.Size() != 0 )
		{
			// rotation
			const quat_t& q0 = banim.keyframes[l_pose].rotation;
			const quat_t& q1 = banim.keyframes[r_pose].rotation;
			local_rot = q0.Slerp(q1, t);


			// translation
			const vec3_t& v0 = banim.keyframes[l_pose].translation;
			const vec3_t& v1 = banim.keyframes[r_pose].translation;
			local_transl = v0.Lerp( v1, t );

			//mat4_t m4 = bone.matrix_as * mat4_t(local_transl) * mat4_t(local_rot) * bone.matrix_as.Inverted();
//			mat4_t m4 = bone.matrix_as * mat4_t(local_rot) * bone.matrix_as.Inverted();
//			mat4_t m4_ = bone.matrix_as * mat4_t(local_transl) * bone.matrix_as.Inverted();
//			local_rot = quat_t( m4.GetRotationPart() );
//			local_transl = m4.GetTranslationPart() + m4_.GetTranslationPart();

//			quat_t q[2];
//			vec3_t v[2];
//			q[0] = banim.keyframes[l_pose].rotation;
//			q[1] = banim.keyframes[r_pose].rotation;
//			v[0] = banim.keyframes[l_pose].translation;
//			v[1] = banim.keyframes[r_pose].translation;
//
//			for( int i=0; i<2; i++ )
//			{
//				//m4 = bone.matrix_as * mat4_t( v[i] ) * mat4_t( q[i] ) * bone.matrix_as.Inverted();
//				mat3_t m3 = bone.matrix_as.GetRotationPart() * mat3_t(q[i]) * bone.matrix_as.Inverted().GetRotationPart();
//				q[i] = quat_t( m3 );
//				v[i] = bone.matrix_as * v[i];
//			}
//			local_rot = q[0].Slerp(q[1], t);
//			local_transl = v[0].Lerp( v[1], t );
		}
		else
		{
			local_rot.SetIdent();
			local_transl.SetZero();
		}

		//bone_poses[bone_id].total = bone.matrix_as * mat4_t(local_transl) * mat4_t(local_rot) * bone.matrix_as.Inverted();
		//bone_poses[bone_id].total = mat4_t(local_transl) * ( mat4_t(bone.head_as) * mat4_t(local_rot) * mat4_t(-bone.head_as) );
		//bone_poses[bone_id].total = MA * mat4_t(local_transl) * mat4_t(local_rot) * MA.Inverted();


		//bones_state[bone_id].transformation = mat4_t::CombineTransformations( mat4_t::TRS( local_transl, mat3_t(local_rot), 1.0 ), MAi );
		//bones_state[bone_id].transformation = mat4_t::CombineTransformations( MA, bones_state[bone_id].transformation );

		mat3_t m3;
		vec3_t v3;
		CombineTransformations( local_transl, local_rot, bone.tsl_skel_space_inv, bone.rot_skel_space_inv, v3, m3 );
		bones_state[bone_id].transformation = mat4_t::TRS( v3, m3, 1.0 );

		bones_state[bone_id].transformation = mat4_t::CombineTransformations( MA, bones_state[bone_id].transformation );


		// 1: Apply the parent's transformation and then store the pose
		if( bone.parent )
		{
			//bone_poses[bone_id].rotation = bone_poses[bone.parent->id].rotation * local_rot;
			//bone_poses[bone_id].translation = bone_poses[bone.parent->id].translation + local_transl;

			bones_state[bone_id].transformation = mat4_t::CombineTransformations( bones_state[bone.parent->id].transformation, bones_state[bone_id].transformation );
		}

		/// end of calculations

		for( int i=0; i<bone.childs_num; i++ )
			queue[tail++] = bone.childs[i]->id;
		++head;
	}
}


/*
=======================================================================================================================================
model_t::Render                                                                                                                       =
=======================================================================================================================================
*/
void model_t::Render()
{
	// transform
	glPushMatrix();
	UpdateWorldTransform();
	glMultMatrixf( &(world_transform.Transposed())(0,0) );

	// material
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_LIGHTING );
	glPolygonMode( GL_FRONT, GL_FILL );

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
		glDisable( GL_DEPTH_TEST );
		for( int i=0; i<model_data->bones.Size(); i++ )
		{
			bone_t& bone = model_data->bones[i];

			//transf = mat4_t::TRS( bone_poses[i].translation, mat3_t(bone_poses[i].rotation), 1.0 );
			//vec3_t head = bone.matrix_as * (transf * vec3_t( 0.0, 0.0, 0.0 ));
			//vec3_t tail = bone.matrix_as * (transf * vec3_t( 0.0, bone.length, 0.0 ));
			//vec3_t head = transf * bone.head_as;
			//vec3_t tail = transf * bone.tail_as;
			vec3_t head = bones_state[i].transformation * bone.head_as;
			vec3_t tail = bones_state[i].transformation * bone.tail_as;

			glPointSize( 4.0f );
			glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
			glBegin( GL_POINTS );
				glVertex3fv( &head[0] );
			glEnd();

			glBegin( GL_LINES );
				glVertex3fv( &head[0] );
				glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
				glVertex3fv( &tail[0] );
			glEnd();
		}
	}

	// end
	glPopMatrix();
}


/*
=======================================================================================================================================
model_t::Deform                                                                                                                       =
=======================================================================================================================================
*/
void model_t::Deform()
{
	memcpy( &verts[0], &model_data->verts[0], sizeof(vertex_t)*model_data->verts.Size() );
}




























