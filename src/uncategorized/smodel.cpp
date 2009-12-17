#include <fstream>
#include "smodel.h"
#include "resource.h"
#include "material.h"
#include "renderer.h"
#include "scanner.h"
#include "parser.h"


/*
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
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;


	//** BONES NUM **
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	bones.resize( token->value.int_, bone_data_t() );

	for( uint i=0; i<bones.size(); i++ )
	{
		bone_data_t& bone = bones[i];
		bone.id = i;

		// NAME
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_STRING )
		{
			PARSE_ERR_EXPECTED( "string" );
			return false;
		}
		bone.SetName( token->value.string );

		// head
		if( !ParseArrOfNumbers<float>( scanner, false, true, 3, &bone.head[0] ) ) return false;

		// tail
		if( !ParseArrOfNumbers<float>( scanner, false, true, 3, &bone.tail[0] ) ) return false;

		// matrix
		mat4_t m4;
		if( !ParseArrOfNumbers<float>( scanner, false, true, 16, &m4[0] ) ) return false;

		// matrix for real
		bone.rot_skel_space = m4.GetRotationPart();
		bone.tsl_skel_space = m4.GetTranslationPart();
		mat4_t MAi( m4.Inverted() );
		bone.rot_skel_space_inv = MAi.GetRotationPart();
		bone.tsl_skel_space_inv = MAi.GetTranslationPart();

		// parent
		token = &scanner.GetNextToken();
		if( (token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT) &&
		    (token->code != scanner_t::TC_IDENTIFIER || !strcmp( token->value.string, "REPEAT_TXTR_MAP_T" )) )
		{
			PARSE_ERR_EXPECTED( "integer or NULL" );
			return false;
		}

		if( token->code == scanner_t::TC_IDENTIFIER )
			bone.parent = NULL;
		else
			bone.parent = &bones[ token->value.int_ ];

		// childs
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}
		if( token->value.int_ > bone_data_t::MAX_CHILDS_PER_BONE )
		{
			ERROR( "Childs for bone \"" << bone.GetName() << "\" exceed the max" );
			return false;
		}
		bone.childs_num = token->value.int_;
		for( int j=0; j<bone.childs_num; j++ )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			bone.childs[j] = &bones[ token->value.int_ ];
		}
	}

	return true;
}


/*
=======================================================================================================================================
model_data_t::LoadVWeights                                                                                                            =
=======================================================================================================================================
*/
/*bool model_data_t::LoadVWeights( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;


	// get the verts num
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	// do a check
	if( token->value.int_ != mesh_data->vert_coords.size() )
	{
		ERROR( "The verts weight num is not equal to the verts num" );
		return false;
	}
	vert_weights.resize( token->value.int_ );

	// for all vert weights
	for( uint i=0; i<vert_weights.size(); i++ )
	{
		// get the bone connections num
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}

		// we treat as error if one vert doesnt have a bone
		if( token->value.int_ < 1 )
		{
			ERROR( "Vert \"" << i << "\" doesnt have at least one bone" );
			return false;
		}

		// and here is another possible error
		if( token->value.int_ > vertex_weight_t::MAX_BONES_PER_VERT )
		{
			ERROR( "Cannot have more than " << vertex_weight_t::MAX_BONES_PER_VERT << " bones per vertex" );
			return false;
		}

		vert_weights[i].bones_num = token->value.int_;

		for( uint j=0; j<vert_weights[i].bones_num; j++ )
		{
			// read bone id
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			if( token->value.int_ >= skeleton_data->bones.size() )
			{
				ERROR( "Incorrect bone id" );
				return false;
			}
			vert_weights[i].bone_ids[j] = token->value.int_;

			// read the weight of that bone
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_FLOAT )
			{
				PARSE_ERR_EXPECTED( "float" );
				return false;
			}
			vert_weights[i].weights[j] = token->value.float_;
		}
	}

	return true;
}*/


/*
=======================================================================================================================================
model_data_t::Load                                                                                                                    =
=======================================================================================================================================
*/
bool model_data_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;

	do
	{
		token = &scanner.GetNextToken();

		//** MESH **
		if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "MESH" ) )
		{
			if( mesh_data )
			{
				PARSE_ERR( "mesh_data allready set" );
				return false;
			}

			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			if( (mesh_data = rsrc::mesh_datas.Load( token->value.string )) == NULL ) return false;
		}

		//** SKELETON **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SKELETON" ) )
		{
			if( skeleton_data )
			{
				PARSE_ERR( "skeleton_data allready set" );
				return false;
			}

			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			skeleton_data = new skeleton_data_t;
			if( !skeleton_data->Load( token->value.string ) ) return false;
		}

		//** VERTEX_WEIGHTS **
		/*else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "VERTEX_WEIGHTS" ) )
		{
			if( vert_weights.size() > 0 )
			{
				PARSE_ERR( "vert_weights not empty" );
				return false;
			}

			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			if( !LoadVWeights( token->value.string ) ) return false;
		}*/

		//** EOF **
		else if( token->code == scanner_t::TC_EOF )
		{
			break;
		}

		//** other crap **
		else
		{
			PARSE_ERR_UNEXPECTED();
			return false;
		}
	}while( true ); // end do

	return true;
}



/*
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
	/*fstream file( filename, ios::in );
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

				file >> bones[i].keyframes[j].rotation.w >> bones[i].keyframes[j].rotation.x >> bones[i].keyframes[j].rotation.y >>
				        bones[i].keyframes[j].rotation.z;

				file >> str;
				for( int k=0; k<3; k++ )
					file >> bones[i].keyframes[j].translation[k];
			}
		}
	}

	file.close();
	return true;*/

	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;


	// keyframes
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT  )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	keyframes.resize( token->value.int_ );

	if( !ParseArrOfNumbers( scanner, false, false, keyframes.size(), &keyframes[0] ) ) return false;

	// bones num
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT  )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	bones.resize( token->value.int_ );

	// poses
	for( uint i=0; i<bones.size(); i++ )
	{
		// has anim?
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT  )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}

		// it has
		if( token->value.int_ == 1 )
		{
			bones[i].keyframes.resize( keyframes.size() );

			for( uint j=0; j<keyframes.size(); ++j )
			{
				// parse the quat
				float tmp[4];
				if( !ParseArrOfNumbers( scanner, false, true, 4, &tmp[0] ) ) return false;
				bones[i].keyframes[j].rotation = quat_t( tmp[1], tmp[2], tmp[3], tmp[0] );

				// parse the vec3
				if( !ParseArrOfNumbers( scanner, false, true, 3, &bones[i].keyframes[j].translation[0] ) ) return false;
			}
		}
	} // end for all bones


	frames_num = keyframes[ keyframes.size()-1 ] + 1;

	return true;
}


/*
=======================================================================================================================================
model                                                                                                                                 =
=======================================================================================================================================
*/


/*
=======================================================================================================================================
model_t::Init                                                                                                                         =
=======================================================================================================================================
*/
void smodel_t::Init( model_data_t* model_data_ )
{
	model_data = model_data_;

	// init the bones
	bones.resize( model_data->skeleton_data->bones.size() );
	bone_rotations.resize( model_data->skeleton_data->bones.size() );
	bone_translations.resize( model_data->skeleton_data->bones.size() );

	// init the verts and vert tangents
	vert_coords.resize( model_data->mesh_data->vert_coords.size() );
	vert_normals.resize( model_data->mesh_data->vert_coords.size() );
	vert_tangents.resize( model_data->mesh_data->vert_coords.size() );

	// the material
	material = rsrc::materials.Load( model_data->mesh_data->material_name );
}


/*
=======================================================================================================================================
model_t::Interpolate                                                                                                                  =
=======================================================================================================================================
*/
void smodel_t::Interpolate( skeleton_anim_t* anim, float frame )
{
	DEBUG_ERR( frame >= anim->frames_num );

	// calculate the t (used in slerp and lerp) and
	// calc the l_pose and r_pose witch indicate the pose ids in witch the frame lies between
	const vec_t<uint>& keyframes = anim->keyframes;
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


	uint i;
	// now for all bones update bone's poses
	for( i=0; i<bones.size(); i++ )
	{
		const bone_anim_t& banim = anim->bones[i];

		mat3_t& local_rot = bone_rotations[i];
		vec3_t& local_transl = bone_translations[i];

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if( banim.keyframes.size() != 0 )
		{
			const bone_pose_t& l_bpose = banim.keyframes[l_pose];
			const bone_pose_t& r_bpose = banim.keyframes[r_pose];

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


/*
=======================================================================================================================================
model_t::Interpolate                                                                                                                  =
=======================================================================================================================================
*/
void smodel_t::Interpolate()
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
void smodel_t::UpdateBoneTransforms()
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


/*
=======================================================================================================================================
model_t::Play                                                                                                                         =
=======================================================================================================================================
*/
void smodel_t::Play( skeleton_anim_t* anim, ushort slot, float step, ushort play_type, float /*smooth_transition_frames*/ )
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
void smodel_t::Deform()
{
	/*DEBUG_ERR( !model_data );

	// deform the verts
	for( uint i=0; i<model_data->mesh_data->vert_coords.size(); i++ ) // for all verts
	{
		const mesh_data_t::vertex_weight_t& vw = model_data->mesh_data->vert_weights[i];

		// a small optimazation detour in case the vert has only one bone
		if( vw.bones_num == 1 )
		{
			const uint& bid = vw.bone_ids[0];
			const mat3_t& rot = bone_rotations[ bid ];
			const vec3_t& transl = bone_translations[ bid ];

			vert_coords[i] = model_data->mesh_data->vert_coords[i].Transformed( transl, rot );
			vert_normals[i] = rot * model_data->mesh_data->vert_normals[i];
			vert_tangents[i] = vec4_t( rot * vec3_t(model_data->mesh_data->vert_tangents[i]), model_data->mesh_data->vert_tangents[i].w );
			continue;
		}

		// calc the matrix according the weights
		mat3_t m3( 0.0 );
		vec3_t v3( 0.0 );
		// for all bones of this vert
		for( int j=0; j< vw.bones_num; j++ )
		{
			uint bid = vw.bone_ids[j];

			m3 += bone_rotations[ bid ] * vw.weights[j];
			v3 += bone_translations[ bid ] * vw.weights[j];
		}

		// apply the matrix to the verts
		vert_coords[i] = model_data->mesh_data->vert_coords[i].Transformed( v3, m3 );
		vert_normals[i] = m3 * model_data->mesh_data->vert_normals[i];
		vert_tangents[i] = vec4_t( m3 * vec3_t(model_data->mesh_data->vert_tangents[i]), model_data->mesh_data->vert_tangents[i].w );
	}*/


	// deform the heads and tails
	if( r::dbg::show_skeletons )
	{
		for( uint i=0; i<model_data->skeleton_data->bones.size(); i++ )
		{
			const mat3_t& rot = bone_rotations[ i ];
			const vec3_t& transl = bone_translations[ i ];

			bones[i].head = model_data->skeleton_data->bones[i].head.Transformed( transl, rot );
			bones[i].tail = model_data->skeleton_data->bones[i].tail.Transformed( transl, rot );
		}
	}
}


/*
=======================================================================================================================================
model_t::RenderSkeleton                                                                                                               =
=======================================================================================================================================
*/
void smodel_t::RenderSkeleton()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	//glPointSize( 4.0f );
	//glLineWidth( 2.0f );

	for( uint i=0; i<model_data->skeleton_data->bones.size(); i++ )
	{
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

	glPopMatrix();
}


/*
=======================================================================================================================================
model_t::Render                                                                                                                       =
=======================================================================================================================================
*/
void smodel_t::Render()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	/*GLuint loc = material->tangents_attrib_loc;

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableVertexAttribArray( loc );

	glVertexPointer( 3, GL_FLOAT, 0, &vert_coords[0][0] );
	glNormalPointer( GL_FLOAT, 0, &vert_normals[0][0] );
	glTexCoordPointer( 2, GL_FLOAT, 0, &model_data->mesh_data->tex_coords[0] );
	glVertexAttribPointer( loc, 4, GL_FLOAT, 0, 0, &vert_tangents[0] );

	glDrawElements( GL_TRIANGLES, model_data->mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, &model_data->mesh_data->vert_indeces[0] );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableVertexAttribArray( loc );*/

	glUniformMatrix3fv( material->skinning_rotations_uni_loc, model_data->skeleton_data->bones.size(), 1, &(bone_rotations[0])[0] );
	glUniform3fv( material->skinning_translations_uni_loc, model_data->skeleton_data->bones.size(), &(bone_translations[0])[0] );


	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );


	/*glVertexPointer( 3, GL_FLOAT, 0, &model_data->mesh_data->vert_coords[0][0] );
	glNormalPointer( GL_FLOAT, 0, &model_data->mesh_data->vert_normals[0][0] );
	glTexCoordPointer( 2, GL_FLOAT, 0, &model_data->mesh_data->tex_coords[0] );

	glEnableVertexAttribArray( material->tangents_attrib_loc );
	glVertexAttribPointer( material->tangents_attrib_loc, 4, GL_FLOAT, 0, 0, &model_data->mesh_data->vert_tangents[0] );

	glEnableVertexAttribArray( material->vert_weight_bones_num_attrib_loc );
	glVertexAttribPointer( material->vert_weight_bones_num_attrib_loc, 1, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), &model_data->mesh_data->vert_weights[0].bones_num );
	glEnableVertexAttribArray( material->vert_weight_bone_ids_attrib_loc );
	glVertexAttribPointer( material->vert_weight_bone_ids_attrib_loc, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), &model_data->mesh_data->vert_weights[0].bone_ids[0] );
	glEnableVertexAttribArray( material->vert_weight_weights_attrib_loc );
	glVertexAttribPointer( material->vert_weight_weights_attrib_loc, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), &model_data->mesh_data->vert_weights[0].weights[0] );

	glDrawElements( GL_TRIANGLES, model_data->mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, &model_data->mesh_data->vert_indeces[0] );*/


	/////////////////////////////////////////////////
	glEnableClientState( GL_VERTEX_ARRAY );
	model_data->mesh_data->vbos.vert_coords.Bind();
	glVertexPointer( 3, GL_FLOAT, 0, NULL );

	glEnableClientState( GL_NORMAL_ARRAY );
	model_data->mesh_data->vbos.vert_normals.Bind();
	glNormalPointer( GL_FLOAT, 0, NULL );

	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	model_data->mesh_data->vbos.tex_coords.Bind();
	glTexCoordPointer( 2, GL_FLOAT, 0, NULL );

	glEnableVertexAttribArray( material->tangents_attrib_loc );
	model_data->mesh_data->vbos.vert_tangents.Bind();
	glVertexAttribPointer( material->tangents_attrib_loc, 4, GL_FLOAT, false, 0, NULL );


	model_data->mesh_data->vbos.vert_weights.Bind();
	glEnableVertexAttribArray( material->vert_weight_bones_num_attrib_loc );
	glVertexAttribPointer( material->vert_weight_bones_num_attrib_loc, 1, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), BUFFER_OFFSET(0) );
	glEnableVertexAttribArray( material->vert_weight_bone_ids_attrib_loc );
	glVertexAttribPointer( material->vert_weight_bone_ids_attrib_loc, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), BUFFER_OFFSET(4) );
	glEnableVertexAttribArray( material->vert_weight_weights_attrib_loc );
	glVertexAttribPointer( material->vert_weight_weights_attrib_loc, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), BUFFER_OFFSET(20) );


	model_data->mesh_data->vbos.vert_indeces.Bind();
	glDrawElements( GL_TRIANGLES, model_data->mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );

	vbo_t::UnbindAllTargets();
	/////////////////////////////////////////////////



	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableVertexAttribArray( material->tangents_attrib_loc );
	glDisableVertexAttribArray( material->vert_weight_bones_num_attrib_loc );


	glPopMatrix();
}


/*
=======================================================================================================================================
model_t::RenderDepth                                                                                                                  =
=======================================================================================================================================
*/
void smodel_t::RenderDepth()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );


	glUniformMatrix3fv( r::is::shadows::shdr_depth_hw_skinning->GetUniformLocation(0), model_data->skeleton_data->bones.size(), 1, &(bone_rotations[0])[0] );
	glUniform3fv( r::is::shadows::shdr_depth_hw_skinning->GetUniformLocation(1), model_data->skeleton_data->bones.size(), &(bone_translations[0])[0] );


	model_data->mesh_data->vbos.vert_coords.Bind();
	glVertexPointer( 3, GL_FLOAT, 0, NULL );
	glEnableClientState( GL_VERTEX_ARRAY );

	model_data->mesh_data->vbos.vert_weights.Bind();
	glEnableVertexAttribArray( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(0) );
	glVertexAttribPointer( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(0), 1, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), BUFFER_OFFSET(0) );
	glEnableVertexAttribArray(r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(1) );
	glVertexAttribPointer( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(1), 4, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), BUFFER_OFFSET(4) );
	glEnableVertexAttribArray( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(2) );
	glVertexAttribPointer( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(2), 4, GL_FLOAT, GL_FALSE, sizeof(mesh_data_t::vertex_weight_t), BUFFER_OFFSET(20) );

	model_data->mesh_data->vbos.vert_indeces.Bind();


	glDrawElements( GL_TRIANGLES, model_data->mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );

	glDisableClientState( GL_VERTEX_ARRAY );

	vbo_t::UnbindAllTargets();

	glPopMatrix();
}


























