#include <iostream>
#include <fstream>
#include "geometry.h"
#include "shaders.h"
using namespace std;

/*
=================================================================================================================================================================================
CalcFaceNormals                                                                                                                                                 =
=================================================================================================================================================================
*/
void mesh_data_t::CalcFaceNormals()
{
	for( int i=0; i<tris.size; i++ )
	{
		triangle_t& tri = tris[i];
		const vec3_t& v0 = verts[ tri.vert_ids[0] ].coords;
		const vec3_t& v1 = verts[ tri.vert_ids[1] ].coords;
		const vec3_t& v2 = verts[ tri.vert_ids[2] ].coords;

		tri.normal = ( v1 - v0 ) * ( v2 - v1 );

		tri.normal.Normalize();
	}
}


/*
=================================================================================================================================================================
CalcVertNormals                                                                                                                                                 =
=================================================================================================================================================================
*/
void mesh_data_t::CalcVertNormals()
{
	for( int i=0; i<verts.size; i++ )
		verts[i].normal.LoadZero();

	for( int i=0; i<tris.size; i++ )
	{
		triangle_t& tri = tris[i];
		verts[ tri.vert_ids[0] ].normal += tri.normal;
		verts[ tri.vert_ids[1] ].normal += tri.normal;
		verts[ tri.vert_ids[2] ].normal += tri.normal;
	}

	for( int i=0; i<verts.size; i++ )
		verts[i].normal.Normalize();
}



/*
=================================================================================================================================================================================
CalcFaceTangents                                                                                                                                                =
=================================================================================================================================================================
*/
void mesh_data_t::CalcFaceTangents()
{
	for( int i=0; i<tris.size; i++ )
	{
		const triangle_t& tri = tris[i];
		const vec3_t& v0 = verts[ tri.vert_ids[0] ].coords;
		const vec3_t& v1 = verts[ tri.vert_ids[1] ].coords;
		const vec3_t& v2 = verts[ tri.vert_ids[2] ].coords;
		vec3_t edge01 = v1 - v0;
		vec3_t edge02 = v2 - v0;

		vec2_t uvedge01 = uvs[i][1] - uvs[i][0];
		vec2_t uvedge02 = uvs[i][2] - uvs[i][0];

		uvedge01.Normalize();
		uvedge02.Normalize();


		float det = (uvedge01.X * uvedge02.Y) - (uvedge01.Y * uvedge02.X);
		DEBUG_ERR( IsZero(det) );
		det = 1.0f / det;

		vec3_t t = ( edge01 * uvedge02.Y - edge02 * uvedge01.Y ) * det;
		vec3_t b = ( edge02 * uvedge01.X - edge01 * uvedge02.X ) * det;
		t.Normalize();
		b.Normalize();

		//face_tangents[i] = t;

		vec3_t bitangent( tri.normal * t );
    float handedness = ( bitangent.Dot( b ) < 0.0f) ? -1.0f : 1.0f;

		face_tangents[i] = vec4_t( t.X, t.Y, t.Z, handedness );

	}

}


/*
=================================================================================================================================================================================
CalcVertTangents                                                                                                                                                =
=================================================================================================================================================================
*/
void mesh_data_t::CalcVertTangents()
{
	for( int i=0; i<vert_tangents.size; i++ )
		vert_tangents[i].LoadZero();

	for( int i=0; i<tris.size; i++ )
	{
		const vec4_t& ft = face_tangents[i];
		const triangle_t& tri = tris[i];

		vert_tangents[ tri.vert_ids[0] ] += ft;
		vert_tangents[ tri.vert_ids[1] ] += ft;
		vert_tangents[ tri.vert_ids[2] ] += ft;
	}

	for( int i=0; i<vert_tangents.size; i++ )
	{
		/*vec3_t v3( vert_tangents[i].X, vert_tangents[i].Y, vert_tangents[i].Z );
		v3.Normalize();
		vert_tangents[i] = vec4_t( v3.X, v3.Y, v3.Z, vert_tangents[i].W );*/

		vert_tangents[i] = vec4_t( vec3_t(vert_tangents[i]).Normalized(), vert_tangents[i].W );
	}
//    vec3_t* tan1 = new vec3_t[verts.size * 2];
//    vec3_t* tan2 = tan1 + verts.size;
//    ZeroMemory(tan1, vertexCount * sizeof(Vector3D) * 2);
//    memset( tan1, 0, verts.size * 2 );
//
//    for (long a = 0; a < tris.size; a++)
//    {
//			triangle_t* triangle = &tris[a];
//        long i1 = triangle->ids[0];
//        long i2 = triangle->ids[1];
//        long i3 = triangle->ids[2];
//
//        const vec3_t& v1 = verts[i1];
//        const vec3_t& v2 = verts[i2];
//        const vec3_t& v3 = verts[i3];
//
//        const vec2_t& w1 = uvs texcoord[i1];
//        const vec2_t& w2 = texcoord[i2];
//        const vec2_t& w3 = texcoord[i3];
//
//        float x1 = v2.x - v1.x;
//        float x2 = v3.x - v1.x;
//        float y1 = v2.y - v1.y;
//        float y2 = v3.y - v1.y;
//        float z1 = v2.z - v1.z;
//        float z2 = v3.z - v1.z;
//
//        float s1 = w2.x - w1.x;
//        float s2 = w3.x - w1.x;
//        float t1 = w2.y - w1.y;
//        float t2 = w3.y - w1.y;
//
//        float r = 1.0F / (s1 * t2 - s2 * t1);
//        Vector3D sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
//                (t2 * z1 - t1 * z2) * r);
//        Vector3D tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
//                (s1 * z2 - s2 * z1) * r);
//
//        tan1[i1] += sdir;
//        tan1[i2] += sdir;
//        tan1[i3] += sdir;
//
//        tan2[i1] += tdir;
//        tan2[i2] += tdir;
//        tan2[i3] += tdir;
//
//        triangle++;
//    }
//
//    for (long a = 0; a < vertexCount; a++)
//    {
//        const Vector3D& n = normal[a];
//        const Vector3D& t = tan1[a];
//
//        // Gram-Schmidt orthogonalize
//        tangent[a] = (t - n * Dot(n, t)).Normalize();
//
//        // Calculate handedness
//        tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
//    }
//
//    delete[] tan1;

}


/*
=================================================================================================================================================================
Load                                                                                                                                                            =
=================================================================================================================================================================
*/
int mesh_data_t::Load( const char* filename )
{
	fstream file( filename, ios::in );
	char str[400];
	int i, num;

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return 0;
	}

	// verts
	file >> str >> num;
	verts.Malloc( num );

	for( i=0; i< verts.size; i++ )
	{
		file >> str >> str >> str >> verts[i].coords.X >> verts[i].coords.Y >> verts[i].coords.Z;
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
		file >> str >> str >> uvs[i][0].X >> uvs[i][0].Y >> uvs[i][1].X >> uvs[i][1].Y >> uvs[i][2].X >> uvs[i][2].Y;
	}

	CalcAllNormals();

	face_tangents.Malloc( tris.size );
	vert_tangents.Malloc( verts.size );
	CalcAllTangents();

	// populate vert list
	vert_list.Malloc( tris.size * 3 );
	for( i=0; i<tris.size; i++ )
	{
		vert_list[i*3+0] = tris[i].vert_ids[0];
		vert_list[i*3+1] = tris[i].vert_ids[1];
		vert_list[i*3+2] = tris[i].vert_ids[2];
	}

	file.close();

	return 1;
}


/*
=================================================================================================================================================================
Render                                                                                                                                                          =
=================================================================================================================================================================
*/
void mesh_t::Render()
{
	int i, j;


	// set gl state
	glPolygonMode( GL_FRONT, GL_FILL );
	glEnable( GL_LIGHTING );        // remove if pixel shaders enabled
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	//glEnable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );


	// transform
	glPushMatrix();
	UpdateWorldTransform();
	mat4_t transf( TRS( world_translate, world_rotate, world_scale) );
	transf.Transpose();
	glMultMatrixf( &transf(0,0) );

	// render
	extern shader_prog_t shaderp;
	GLuint loc = shaderp.GetAttrLocation( "tangent" );

	glBegin( GL_TRIANGLES );
		for( i=0; i<mesh_data->tris.size; i++ )
		{
			triangle_t& tri = mesh_data->tris[i];

			//glNormal3fv( &tri.normal[0] );
			//glVertexAttrib4fv( loc, &mesh_data->face_tangents[ i ][0] );

			vec2_t* uv = mesh_data->uvs[i];
			for( j=0; j<3; j++ )
			{
				const int vert_id = tri.vert_ids[j];
				vertex_t& vert = mesh_data->verts[ vert_id ];

				//glVertexAttrib4fv( loc, &mesh_data->vert_tangents[ vert_id ][0] );
				glNormal3fv( &vert.normal[0] );

				glTexCoord2fv( &uv[j][0] );
				glVertex3fv( &vert.coords[0] );
			}
		}
	glEnd();


	r::NoShaders();

	// vert normals
	if( 0 )
	{
		glColor3f( 0.0, 0.0, 1.0 );
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );

		glBegin( GL_LINES );
			for( i=0; i<mesh_data->tris.size; i++ )
			{
				triangle_t* tri = &mesh_data->tris[i];
				for( j=0; j<3; j++ )
				{
					vertex_t* vert = &mesh_data->verts[tri->vert_ids[j]];

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
		glLineWidth( 1.0f );

		glBegin( GL_LINES );
			for( i=0; i<mesh_data->tris.size; i++ )
			{
				triangle_t* tri = &mesh_data->tris[i];

				vec3_t vec1;
				vec1 = mesh_data->verts[ tri->vert_ids[0] ].coords;
				vec1 = ( vec1 + mesh_data->verts[ tri->vert_ids[1] ].coords ) / 2.0f;
				vec1 = ( vec1 + mesh_data->verts[ tri->vert_ids[2] ].coords ) / 2.0f;

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

	// render axis
	if( 0 ) RenderAxis();

	glPopMatrix();
}























