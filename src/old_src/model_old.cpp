#include <iostream>
#include <fstream>
#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>
#include "model.h"
#include "local.h"
#include "math.h"
#include <math.h>

using namespace std;


/*
==========
mesh_t::Load
==========
*/
int mesh_t::Load( char* filename )
{
	fstream file( filename, ios::in );
	char str[200];
	int i, num;

	if( !file.is_open() )
	{
		cout << "-->ERROR (mesh_t::Load): Cannot open \"" << filename << "\"" << endl;
		return 0;
	}

	// verts
	file >> str >> num;
	verts.Malloc( num );

	for( i=0; i< verts.size; i++ )
	{
		file >> str >> str >> str >> verts[i].coords.x() >> verts[i].coords.y() >> verts[i].coords.z();
		//      VERT   index  COORDS num                     num                     num                     NORMAL num                     num                     num
	}

	// tris
	file >> str >> num;
	tris.Malloc( num );

	for( i=0; i<tris.size; i++ )
	{
		file >> str >> str >> str >> tris[i].vert_ids[0] >> tris[i].vert_ids[1] >> tris[i].vert_ids[2];
	}

	// uvs
	file >> str >> num;
	uvs.Malloc( num );

	for( i=0; i<uvs.size; i++ )
	{
		file >> str >> str >> uvs[i][0].x() >> uvs[i][0].y() >> uvs[i][1].x() >> uvs[i][1].y() >> uvs[i][2].x() >> uvs[i][2].y();
	}

	// groups
	file >> str >> num;
	vert_groups.Malloc( num );

	for( i=0; i<vert_groups.size; i++ )
		file >> str >> str >> str >> vert_groups[i].name;

	// weights
	file >> str >> num;
	weights.Malloc( num );

	for( i=0; i<weights.size; i++ )
	{
		file >> str >> str >> str >> str >> str >> num; // pairs num
		weights[i].pairs.Malloc( num );
		for( int j=0; j<weights[i].pairs.size; j++ )
			file >> str >> weights[i].pairs[j].vert_group_id >> str >> weights[i].pairs[j].weight;
	}

	CalcAllNormals();
	file.close();

	return 1;
}

/*
==========
mesh_t::CalcFaceNormals
==========
*/
void mesh_t::CalcFaceNormals()
{
	int i;
	vec3_t a, b;
	triangle_t* tri;

	for( i=0; i<tris.size; i++ )
	{
		tri = &tris[i];

		a = verts[ tri->vert_ids[0] ].coords - verts[ tri->vert_ids[1] ].coords;
		b = verts[ tri->vert_ids[1] ].coords - verts[ tri->vert_ids[2] ].coords;
		tri->normal = a*b;
		tri->normal.Normalize();
	}
}


/*
==========
mesh_t::CalcVertNormals
==========
*/
void mesh_t::CalcVertNormals()
{
	int i, j;
	vertex_t* vert;
	triangle_t* tri;
	int found_num;

	// the slow way
	if( vert_tris.size == 0 )
	{

		for( i=0; i<verts.size; i++ )
		{
			vert = &verts[i];
			vert->normal.LoadZero();
			found_num = 0;

			for( j=0; j<tris.size; j++ )
			{
				tri = &tris[j];
				if( tri->vert_ids[0]==i || tri->vert_ids[1]==i || tri->vert_ids[2]==i )
				{
					vert->normal = vert->normal + tri->normal;
					++found_num;
				}
			}
			vert->normal *= 1.0f/(float)found_num;
		}
	}
	// the fast way
	else
	{
		for( i=0; i<verts.size; i++ )
		{
			vert = &verts[i];
			vert->normal.LoadZero();

			for( j=0; j<vert_tris[i].tri_ids.size; j++ ) // for all the tris of this vert
			{
				vert->normal += tris[ vert_tris[i].tri_ids[j] ].normal;
			}
			vert->normal *= 1.0f/(float)vert_tris[i].tri_ids.size; // ToDo:
		} // end for all verts

	}// end else
}


/*
EnableFastNormalCalcs

this is for fast vertex normal calcs but it allocates memory for that
*/
void mesh_t::EnableFastNormalCalcs()
{
	int i, j;
	int arr [40]; // max 40 tris per vert
	int tris_num;
	triangle_t* tri;

	// alloc the main array
	vert_tris.Malloc( verts.size );

	for( i=0; i<verts.size; i++ ) // for all verts
	{
		tris_num = 0;
		for( j=0; j<tris.size; j++ ) // for all tris
		{
			tri = &tris[j];
			if( tri->vert_ids[0]==i || tri->vert_ids[1]==i || tri->vert_ids[2]==i ) // if the vert is in this tri...
			{
				arr[tris_num++] = j; // ...then put it on the arr
			}
		}

		// evaluate the findings for the vert
		vert_tris[i].tri_ids.Malloc( tris_num );
		for( j=0; j<tris_num; j++ )
			vert_tris[i].tri_ids[j] = arr[j];
	}
}


/*
==========
DrawMesh
==========
*/
void DrawMesh( mesh_t& mesh, GLuint tex_id )
{
	int i, j;

	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glColor3fv( &vec3_t(1.0f, 1.0f, 1.0f)[0] );
	glBindTexture(GL_TEXTURE_2D, tex_id);


	/*glBegin( GL_TRIANGLES );
		for( i=0; i<mesh.tris.size; i++ )
		{
			triangle_t* tri = &mesh.tris[i];
			for( j=0; j<3; j++ )
			{
				vertex_t* vert = &mesh.verts[tri->vert_ids[j]];

				glNormal3fv( &vert->normal[0] );
				glTexCoord2fv( tri->uvs[j] );
				glVertex3fv( vert->coords );
			}
		}
	glEnd();*/

	/*glVertexPointer( 3, GL_FLOAT, sizeof(vertex_t), &mesh.verts[0].coords[0] );
	glNormalPointer( GL_FLOAT, sizeof(vertex_t), &mesh.verts[0].normal[0] );*/

	//glDisable( GL_LIGHTING );

	glBegin( GL_TRIANGLES );

		for( i=0; i<mesh.tris.size; i++ )
		{
			triangle_t* tri = &mesh.tris[i];
			for( j=0; j<3; j++ )
			{
				vertex_t* vert = &mesh.verts[tri->vert_ids[j]];

				glTexCoord2fv( &mesh.uvs[i][j][0] );
				glNormal3fv( &vert->normal[0] );
				glVertex3fv( &vert->coords[0] );
				//glArrayElement( mesh.tris[i].vert_ids[j] );
			}
		}

	glEnd();

	// vert normals
	if( 0 )
	{
		glColor3f( 0.0, 0.0, 1.0 );
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );

		glBegin( GL_LINES );
			for( i=0; i<mesh.tris.size; i++ )
			{
				triangle_t* tri = &mesh.tris[i];
				for( j=0; j<3; j++ )
				{
					vertex_t* vert = &mesh.verts[tri->vert_ids[j]];

					vec3_t vec0;
					vec0 = (vert->normal * 0.1f) + vert->coords;

					glVertex3fv( &vert->coords[0] );
					glVertex3fv( &vec0[0] );
				}
			}
		glEnd();
	}

	// tri normals
	if( 0 )
	{
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );

		glBegin( GL_LINES );
			for( i=0; i<mesh.tris.size; i++ )
			{
				triangle_t* tri = &mesh.tris[i];

				vec3_t vec1;
				vec1 = mesh.verts[ tri->vert_ids[0] ].coords;
				vec1 = ( vec1 + mesh.verts[ tri->vert_ids[1] ].coords ) / 2.0f;
				vec1 = ( vec1 + mesh.verts[ tri->vert_ids[2] ].coords ) / 2.0f;

				vec3_t vec2( tri->normal );
				vec2 = tri->normal;
				vec2 *= 0.09;
				vec2 += vec1;

				glColor3f( 0.0, 1.0, 0.0 );
				glVertex3fv( &vec1[0] );
				glColor3f( 0.0, 0.0, 1.0 );
				glVertex3fv( &vec2[0] );
			}
		glEnd();
	}
	/*glEnable( GL_LIGHTING );
	glEnable( GL_DEPTH_TEST );
	glLineWidth( lw );*/


}


/*
==============
armature_t::Load
==============
*/
int armature_t::Load( char* filename )
{
	fstream file( filename, ios::in );
	char str[200];
	int i, j;

	if( !file.is_open() )
	{
		cout << "-->ERROR (armature_t::Load): Cannot open \"" << filename << "\"" << endl;
		return 0;
	}

	// bones heads and tails
	int bones_num;
	file >> str >> bones_num;
	bones.Malloc( bones_num );

	for( i=0; i<bones.size; i++ )
	{
		bone_t* bone = &bones[i];
		// name
		file >> str >> str >> str >> &bone->name[0];

		// head and tail
		file >> str;
		for( j=0; j<3; j++ )
			file >> bone->head[j];

		file >> str;
		for( j=0; j<3; j++ )
			file >> bone->tail[j];

		// parent
		int parent;
		file >> str >> parent;
		bone->parent_id = parent;

		// childs
		int childs_num;
		file >> str >> childs_num >> str;
		bone->child_ids.Malloc( childs_num );
		for( int j=0; j<bone->child_ids.size; j++ )
		{
			int child;
			file >> child;
			bone->child_ids[j] = child;
		}

		// init the matrix
		bone->matrix.LoadIdent();
	}

	file.close();
	return 1;
}


/*
==============
armature_t::Draw
==============
*/
void armature_t::Draw()
{
	float lw;
	float ps;
	vec3_t newhead, newtail;

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glGetFloatv( GL_LINE_WIDTH, &lw );
	glGetFloatv ( GL_POINT_SIZE, &ps );
	glLineWidth( 1.0 );
	glPointSize( 4.0 );

	for( int i=0; i<bones.size; i++ )
	{
		newhead = bones[i].matrix * bones[i].head;
		newtail = bones[i].matrix * bones[i].tail;
		//bone_t* bone = &bones[i];
		//vec3_t h, t, H, T, h_, t_;
		//mat4_t rot, ma, mb, mai;
		//h_.LoadZero();
		//t_.LoadZero(); t_.y() = bones[i].length;
		//H = bones[i].head_armat;
		//T = bones[i].tail_armat;
		//ma = bones[i].matrix_armat;
		//mai = ma; mai.Invert();
		//mb.LoadMat3( bones[i].matrix_bone );
		//h = bones[i].head_bone;
		//t = bones[i].tail_bone;
		//rot.LoadQuat( bone->quat );
		//
		//newhead = ma * rot * mai * H;
		//newtail = ma * rot * mai * T;
		//
		///*if( i==2 )
		//{
		//	mat4_t m4;
		//	m4.LoadMat3( bones[i].matrix_bone );
		//	newhead = rot * h;
		//	newtail = rot * t;
		//}*/

		// draw
		glBegin( GL_POINTS );
			glColor3f( 1.0, 1.0, 1.0 );
			glVertex3fv( &newhead[0] );
		glEnd();

		glBegin( GL_LINES );

			glColor3f( 1.0, 1.0, 1.0 );
			glVertex3fv( &newhead[0] );
			glColor3f( 1.0, 0.0, 0.0 );
			glVertex3fv( &newtail[0] );
		glEnd();
	}

	glLineWidth( lw );
	glPointSize( ps );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
}



/*
==========
armature_anim_t::Load
==========
*/
int armature_anim_t::Load( char* filename )
{
	fstream file( filename, ios::in );
	char str[200];
	int i, j, ii, jj;

	if( !file.is_open() )
	{
		cout << "-->ERROR (armature_anim_t::Load): Cannot open \"" << filename << "\"" << endl;
		return 0;
	}

	// frames num
	file >> str >> frames_num;

	// keyframes list
	int keyframes_num;
	file >> str >> keyframes_num >> str;
	keyframes.Malloc( keyframes_num );
	for( i=0; i<keyframes.size; i++ )
		file >> keyframes[i];

	// bones num
	file >> str >> i;
	bone_poses.Malloc( i );

	// for all bones
	for( i=0; i<bone_poses.size; i++ )
	{
		file >> str >> str >> str >> str;
		file >> str >> j;
		bone_poses[i].poses.Malloc( j );
		for( j=0; j<bone_poses[i].poses.size; j++ )
		{
			// the matrix
			file >> str >> str >> str >> bone_poses[i].poses[j].keyframe;
			file >> str;

			for( ii=0; ii<4; ii++ )
				for( jj=0; jj<4; jj++ )
					file >> bone_poses[i].poses[j].matrix(ii,jj);

			// the loc
			file >> str;
			for( ii=0; ii<3; ii++ )
				file >> bone_poses[i].poses[j].loc[ii];

			// the quat
			file >> str;
			for( ii=0; ii<4; ii++ )
				file >> bone_poses[i].poses[j].quat[ii];
		}
	}

	file.close();
	return 1;
}


/*
===========
model_t::Load
===========
*/
int model_t::Load( char* filename )
{
	fstream file( filename, ios::in );
	char str[200];
	int i;

	if( !file.is_open() )
	{
		cout << "-->ERROR (model_t::Load): Cannot open \"" << filename << "\"" << endl;
		return 0;
	}

	// mesh
	file >> str >> str;
	mesh.Load(str);
	mesh.EnableFastNormalCalcs();

	verts_initial = mesh.verts;

	// armature
	file >> str >> str;
	armat.Load( str );

	// anims
	int anims_num;
	file >> str >> anims_num;
	anims.Malloc( anims_num );
	for( i=0; i<anims.size; i++ )
	{
		file >> str >> str >> str;
		anims[i].Load( str );
	}

	// vgroup2bone
	vgroup2bone.Malloc( mesh.vert_groups.size );
	for( i=0; i<mesh.vert_groups.size; i++ )
	{
		vgroup2bone[i] = -1;
		for( int j=0; j<armat.bones.size; j++ )
		{
			if( strcmp(mesh.vert_groups[i].name, armat.bones[j].name) == 0 )
			{
				vgroup2bone[i] = j;
				continue;
			}
		}
	}


	// end
	file.close();
	return 1;
}


/*
==========
Interpolate
==========
*/
void model_t::Interpolate( int anim_id, int frame )
{
	// calculate the t (used in slerp and lerp) and
	// calc the l_pose and r_pose witch indicate the pose ids in witch the frame lies between
	array_t<int>& keyframes = anims[anim_id].keyframes;
	float t;
	int l_pose=0, r_pose=0;
	for( int j=0; j<keyframes.size; j++ )
	{
		if( keyframes[j] == frame )
		{
			l_pose = r_pose = j;
			break;
		}
		else if( keyframes[j] > frame )
		{
			l_pose = j-1;
			r_pose = j;
			break;
		}
	}
	if( l_pose==r_pose )
		t = 0.0;
	else
		t = float( frame - keyframes[l_pose] ) / float( keyframes[r_pose] - keyframes[l_pose] );


	// using the in depth search algorithm animate the bones
	// from father to child cause the child needs tha father's transformation matrix
	int queue [100]; // the queue for the seatch
	int he = 0, ta = 0; // head and tail of queue

	// put the roots (AKA the bones without father) in the queue
	for( int i=0; i<anims[anim_id].bone_poses.size; i++ )
	{
		if( armat.bones[i].parent_id == -1 )
			queue[ta++] = i;
	}

	// while queue is not empty
	while( he!=ta )
	{
		int bone_id = queue[ he ];
		bone_t* bone = &armat.bones[bone_id];
		bone_poses_t* anim = &anims[anim_id].bone_poses[bone_id];


		// =========================
		// here we do the calculations of the bone's new matrix
		// =========================

		// 0
		// the rotation
		mat4_t m4rot, origin, origini;
		quat_t q;
		vec3_t head_i; // the inverce of bone->head vector

		q.Slerp( anim->poses[l_pose].quat, anim->poses[r_pose].quat, t );
		bone->quat = q;

		mat3_t tmp;
		tmp.LoadQuat( q );
		m4rot.LoadMat3( tmp );

		// m4rot = head * local_rot * inverted(head) AKA rotate from the bone's head
		origin.LoadVec3( bone->head );
		head_i = bone->head;
		head_i *= -1.0;
		origini.LoadVec3( head_i );
		m4rot = origin * m4rot * origini;

		// 1
		// the translation
		vec3_t transv;
		mat4_t m4trans;

		transv.Lerp( anim->poses[l_pose].loc, anim->poses[r_pose].loc, t);
		m4trans.LoadVec3( transv );

		// combine rot and trans
		bone->matrix = m4rot * m4trans;

		// apply the father's transformation
		if( bone->parent_id!=-1 )
			bone->matrix = armat.bones[ bone->parent_id ].matrix * bone->matrix;

		// =========================
		// end animation code
		// =========================


		// queue stuff:
		// put the childs at the end of the queue
		for( int i=0; i<bone->child_ids.size; i++ )
		{
			queue[ta++] = bone->child_ids[i];
		}
		++he;
	}
}


/*
=============
model_t::ApplyArmatAnimToMesh
=============
*/
void model_t::ApplyArmatAnimToMesh()
{
	//int i, j, vert_index, group_index;
	//vertex_group_t* vgroup;
	//
	//for( i=0; i<armat.bones.size; i++ ) // for all the bones
	//{
	//	group_index = armat.bones[i].vert_group_index;
	//	if( group_index==-1 ) continue;
	//
	//	vgroup = &mesh.vert_groups[ group_index ];
	//	for( j=0; j<vgroup->vert_ids.size; j++ )
	//	{
	//		vert_index = vgroup->vert_ids[j];
	//		MatMul( dmatrices[i], initials.vert_coords[vert_index], mesh.verts[ vert_index ].coords );
	//		MatMul( dmatrices[i], initials.vert_norms[vert_index], mesh.verts[ vert_index ].normal );
	//		VecNormalize( mesh.verts[ vert_index ].normal );
	//	}
	//}

	//for( int i=0; i<armat.verts.size; i++ )
	//{
	//	// calc the matrix according the weights
	//	mat4_t m4;
	//	m4.LoadZero();
	//	for( int j=0; j<armat.verts[i].weights_num; j++ )
	//	{
	//		m4 += (armat.bones[ armat.verts[i].bone_indeces[j] ].matrix * armat.verts[i].weights[j]);
	//	}
	//
	//	// apply the matrix to the verts
	//	mesh.verts[ i ].coords = m4 * mesh_basic.verts[i].coords;
	//	mesh.verts[ i ].normal = m4 * mesh_basic.verts[i].normal;
	//	mesh.verts[ i ].normal.Normalize();
	//}

	for( int i=0; i<mesh.verts.size; i++ ) // for all verts
	{
		// calc the matrix according the weights
		mat4_t m4;
		m4.LoadZero();
		for( int j=0; j<mesh.weights[i].pairs.size; j++ ) // for all groups of this vert
		{
			vertex_weights_t* vweights = &mesh.weights[i];

			int vgroup_id = vweights->pairs[j].vert_group_id;
			int bone_id = vgroup2bone[ vgroup_id ];

			if( bone_id == -1 ) continue;

			m4 += (armat.bones[ bone_id ].matrix * vweights->pairs[j].weight);
		}

		// apply the matrix to the verts
		mesh.verts[ i ].coords = m4 * verts_initial[i].coords;
		//mesh.verts[ i ].normal = m4 * mesh_basic.verts[i].normal;
		//mesh.verts[ i ].normal.Normalize();
	}

	mesh.CalcAllNormals();
}
