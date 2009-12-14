#include "geometry.h"
#include "renderer.h"

/*
=================================================================================================================================================================
Load                                                                                                                                                            =
=================================================================================================================================================================
*/
bool mesh_data_t::Load( const char* filename )
{
	fstream file( filename, ios::in );
	char str[400];
	uint i, num;

	if( !file.is_open() )
	{
		ERROR( "Cannot open \"" << filename << "\"" );
		return 0;
	}

	// material. ToDo: add proper code
	file >> str >> str;

	// verts
	file >> str >> num;
	verts.resize( num );

	for( i=0; i< verts.size(); i++ )
	{
		file >> str >> str >> str >> verts[i].coords.x >> verts[i].coords.y >> verts[i].coords.z;
	}

	// tris
	file >> str >> num;
	tris.resize( num );

	for( i=0; i<tris.size(); i++ )
	{
		file >> str >> str >> str >> tris[i].vert_ids[0] >> tris[i].vert_ids[1] >> tris[i].vert_ids[2];
	}

	// uvs
	file >> str >> num;
	uvs.resize( num );

	for( i=0; i<uvs.size(); i++ )
	{
		file >> str >> str >> str >> uvs[i].x >> uvs[i].y;
	}

	CalcAllNormals();

	face_tangents.resize( tris.size() );
	vert_tangents.resize( verts.size() );
	face_bitangents.resize( tris.size() );
	vert_bitangents.resize( verts.size() );
	CalcAllTangents();

	// populate vert list
	vert_list.resize( tris.size() * 3 );
	for( i=0; i<tris.size(); i++ )
	{
		vert_list[i*3+0] = tris[i].vert_ids[0];
		vert_list[i*3+1] = tris[i].vert_ids[1];
		vert_list[i*3+2] = tris[i].vert_ids[2];
	}

	file.close();

	return 1;
}


/*
=================================================================================================================================================================================
CalcFaceNormals                                                                                                                                                 =
=================================================================================================================================================================
*/
void mesh_data_t::CalcFaceNormals()
{
	for( uint i=0; i<tris.size(); i++ )
	{
		triangle_t& tri = tris[i];
		const vec3_t& v0 = verts[ tri.vert_ids[0] ].coords;
		const vec3_t& v1 = verts[ tri.vert_ids[1] ].coords;
		const vec3_t& v2 = verts[ tri.vert_ids[2] ].coords;

		tri.normal = ( v1 - v0 ).Cross( v2 - v0 );

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
	for( uint i=0; i<verts.size(); i++ )
		verts[i].normal.SetZero();

	for( uint i=0; i<tris.size(); i++ )
	{
		triangle_t& tri = tris[i];
		verts[ tri.vert_ids[0] ].normal += tri.normal;
		verts[ tri.vert_ids[1] ].normal += tri.normal;
		verts[ tri.vert_ids[2] ].normal += tri.normal;
	}

	for( uint i=0; i<verts.size(); i++ )
		verts[i].normal.Normalize();
}



/*
=================================================================================================================================================================================
CalcFaceTangents                                                                                                                                                =
=================================================================================================================================================================
*/
void mesh_data_t::CalcFaceTangents()
{
//	for( int i=0; i<tris.size(); i++ )
//	{
//		const triangle_t& tri = tris[i];
//		const int vi0 = tri.vert_ids[0];
//		const int vi1 = tri.vert_ids[1];
//		const int vi2 = tri.vert_ids[2];
//		const vec3_t& v0 = verts[ vi0 ].coords;
//		const vec3_t& v1 = verts[ vi1 ].coords;
//		const vec3_t& v2 = verts[ vi2 ].coords;
//		vec3_t edge01 = v1 - v0;
//		vec3_t edge02 = v2 - v0;
//		vec2_t uvedge01 = uvs[vi1] - uvs[vi0];
//		vec2_t uvedge02 = uvs[vi2] - uvs[vi0];
//
//		float det = (uvedge01.y * uvedge02.x) - (uvedge01.x * uvedge02.y);
//		DEBUG_ERR( IsZero(det) );
//		det = 1.0f / det;
//
//		vec3_t t = ( edge01 * -uvedge02.y + edge02 * uvedge01.y ) * det;
//		vec3_t b = ( edge01 * -uvedge02.x + edge02 * uvedge01.x ) * det;
//		t.Normalize();
//		b.Normalize();
//
//		vec3_t bitangent( tri.normal * t );
//    float handedness = ( bitangent.Dot( b ) < 0.0f) ? -1.0f : 1.0f;
//
//		face_tangents[i] = vec4_t( t.x, t.y, t.z, handedness );
//
//	}


	for( uint i=0; i<tris.size(); i++ )
	{
		const triangle_t& tri = tris[i];
		const int vi0 = tri.vert_ids[0];
		const int vi1 = tri.vert_ids[1];
		const int vi2 = tri.vert_ids[2];
		const vec3_t& v0 = verts[ vi0 ].coords;
		const vec3_t& v1 = verts[ vi1 ].coords;
		const vec3_t& v2 = verts[ vi2 ].coords;
		vec3_t edge01 = v1 - v0;
		vec3_t edge02 = v2 - v0;
		vec2_t uvedge01 = uvs[vi1] - uvs[vi0];
		vec2_t uvedge02 = uvs[vi2] - uvs[vi0];


		float det = (uvedge01.y * uvedge02.x) - (uvedge01.x * uvedge02.y);
		DEBUG_ERR( IsZero(det) );
		det = 1.0f / det;

		vec3_t t = ( edge02 * uvedge01.y - edge01 * uvedge02.y ) * det;
		vec3_t b = ( edge02 * uvedge01.x - edge01 * uvedge02.x ) * det;

		t.Normalize();
		b.Normalize();

		face_tangents[i] = t;
		face_bitangents[i] = b;

	}

}


/*
=================================================================================================================================================================================
CalcVertTangents                                                                                                                                                =
=================================================================================================================================================================
*/
void mesh_data_t::CalcVertTangents()
{
	for( uint i=0; i<vert_tangents.size(); i++ )
		vert_tangents[i].SetZero();

	for( uint i=0; i<tris.size(); i++ )
	{
		const vec3_t& ft = face_tangents[i];
		const vec3_t& fb = face_bitangents[i];
		const triangle_t& tri = tris[i];

		vert_tangents[ tri.vert_ids[0] ] += ft;
		vert_tangents[ tri.vert_ids[1] ] += ft;
		vert_tangents[ tri.vert_ids[2] ] += ft;

		vert_bitangents[ tri.vert_ids[0] ] += fb;
		vert_bitangents[ tri.vert_ids[1] ] += fb;
		vert_bitangents[ tri.vert_ids[2] ] += fb;
	}

	for( uint i=0; i<vert_tangents.size(); i++ )
	{
		vert_tangents[i].Normalize();
		vert_bitangents[i].Normalize();
	}

//    vec3_t* tan1 = new vec3_t[verts.size() * 2];
//    vec3_t* tan2 = tan1 + verts.size();
//    memset( tan1, 0, verts.size() * 2 * sizeof(vec3_t) );
//
//    for (long a = 0; a < tris.size(); a++)
//    {
//			triangle_t* triangle = &tris[a];
//			long i1 = triangle->vert_ids[0];
//			long i2 = triangle->vert_ids[1];
//			long i3 = triangle->vert_ids[2];
//
//			const vec3_t& v1 = verts[i1].coords;
//			const vec3_t& v2 = verts[i2].coords;
//			const vec3_t& v3 = verts[i3].coords;
//
//			const vec2_t& w1 = uvs[i1];
//			const vec2_t& w2 = uvs[i2];
//			const vec2_t& w3 = uvs[i3];
//
//			float x1 = v2.x - v1.x;
//			float x2 = v3.x - v1.x;
//			float y1 = v2.y - v1.y;
//			float y2 = v3.y - v1.y;
//			float z1 = v2.z - v1.z;
//			float z2 = v3.z - v1.z;
//
//			float s1 = w2.x - w1.x;
//			float s2 = w3.x - w1.x;
//			float t1 = w2.y - w1.y;
//			float t2 = w3.y - w1.y;
//
//			float r = 1.0F / (s1 * t2 - s2 * t1);
//			vec3_t sdir( (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
//							(t2 * z1 - t1 * z2) * r);
//			vec3_t tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
//							(s1 * z2 - s2 * z1) * r);
//
//			tan1[i1] += sdir;
//			tan1[i2] += sdir;
//			tan1[i3] += sdir;
//
//			tan2[i1] += tdir;
//			tan2[i2] += tdir;
//			tan2[i3] += tdir;
//
//    }
//
//    for (long a = 0; a < verts.size(); a++)
//    {
//        const vec3_t& n = verts[a].normal;
//        const vec3_t& t = tan1[a];
//
//        // Gram-Schmidt orthogonalize
//        vert_tangents[a] = vec4_t(
//					( (t - n) * n.Dot(t)).Normalized(),
//					( (n*t).Dot( tan2[a] ) < 0.0F) ? -1.0F : 1.0F // handedness
//				);
//    }
//
//    delete[] tan1;
}


/*
=================================================================================================================================================================
Render                                                                                                                                                          =
=================================================================================================================================================================
*/
void mesh_t::Render()
{
	glPushMatrix();
	r::MultMatrix( world_transformation );

	glPolygonMode( GL_FRONT, GL_FILL );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_TEXTURE_2D );


	// render with vert arrays
	//extern shader_prog_t shaderp;
	//GLuint loc = shaderp.GetAttrLocation( "tangent" );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	//glEnableVertexAttribArray( loc );

	glVertexPointer( 3, GL_FLOAT, sizeof(vertex_t), &mesh_data->verts[0].coords[0] );
	glNormalPointer( GL_FLOAT, sizeof(vertex_t), &mesh_data->verts[0].normal[0] );
	glTexCoordPointer( 2, GL_FLOAT, 0, &mesh_data->uvs[0] );
	//glVertexAttribPointer( loc, 4, GL_FLOAT, 0, 0, &mesh_data->vert_tangents[0] );

	glDrawElements( GL_TRIANGLES, mesh_data->vert_list.size(), GL_UNSIGNED_INT, &mesh_data->vert_list[0] );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	//glDisableVertexAttribArray( loc );


	// render
	/*extern shader_prog_t shaderp;
	GLuint loc = shaderp.GetAttrLocation( "tangent" );
	GLuint loc1 = shaderp.GetAttrLocation( "bitangent" ); LALA*/

//	glBegin( GL_TRIANGLES );
//		for( i=0; i<mesh_data->tris.size(); i++ )
//		{
//			triangle_t& tri = mesh_data->tris[i];
//
//			//glNormal3fv( &tri.normal[0] );
//			/*glVertexAttrib3fv( loc, &mesh_data->face_tangents[ i ][0] );
//			glVertexAttrib3fv( loc1, &mesh_data->face_bitangents[ i ][0] ); LALA*/
//
//			for( j=0; j<3; j++ )
//			{
//				const int vert_id = tri.vert_ids[j];
//				vertex_t& vert = mesh_data->verts[ vert_id ];
//
//				//glVertexAttrib3fv( loc, &mesh_data->vert_tangents[ vert_id ][0] );
//				//glVertexAttrib3fv( loc1, &mesh_data->vert_bitangents[ vert_id ][0] );
//				glNormal3fv( &vert.normal[0] );
//				glTexCoord2fv( &mesh_data->uvs[vert_id][0] );
//				glVertex3fv( &vert.coords[0] );
//			}
//		}
//	glEnd();








//
//	r::SetGLState_Wireframe();
//	glColor3fv( &vec3_t(1.0, 1.0, 0.0)[0] );
//	glBegin( GL_TRIANGLES );
//		for( i=0; i<mesh_data->tris.size(); i++ )
//		{
//			triangle_t& tri = mesh_data->tris[i];
//
//			for( j=0; j<3; j++ )
//			{
//				const int vert_id = tri.vert_ids[j];
//				vertex_t& vert = mesh_data->verts[ vert_id ];
//				glVertex3fv( &vert.coords[0] );
//			}
//		}
//	glEnd();



	// tri normals
	if( 0 )
	{
		r::SetGLState_Solid();

		glBegin( GL_LINES );
			for( int i=0; i<mesh_data->tris.size(); i++ )
			{
				const triangle_t& tri = mesh_data->tris[i];

				vec3_t middle = mesh_data->verts[ tri.vert_ids[0] ].coords;
				middle = ( middle + mesh_data->verts[ tri.vert_ids[1] ].coords ) / 2.0f;
				middle = ( middle + mesh_data->verts[ tri.vert_ids[2] ].coords ) / 2.0f;

				vec3_t vec2 = (tri.normal * 0.1) + middle;

				glColor3fv( &vec3_t( 0.0, 0.0, 1.0 )[0] );
				glVertex3fv( &middle[0] );
				glVertex3fv( &vec2[0] );

				vec2 = vec3_t(mesh_data->face_tangents[i] * 0.1) + middle;
				glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
				glVertex3fv( &middle[0] );
				glVertex3fv( &vec2[0] );

				vec2 = vec3_t(mesh_data->face_bitangents[i] * 0.1) + middle;
				glColor3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
				glVertex3fv( &middle[0] );
				glVertex3fv( &vec2[0] );
			}
		glEnd();
	}


	// vert normals
	if( 0 )
	{
		r::NoShaders();
		glColor3f( 0.0, 0.0, 1.0 );
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );

		glBegin( GL_LINES );
			for( int i=0; i<mesh_data->tris.size(); i++ )
			{
				triangle_t* tri = &mesh_data->tris[i];
				for( int j=0; j<3; j++ )
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

	// render axis
	if( 0 ) RenderAxis();

	glPopMatrix();
}























