#include <iostream>
#include <fstream>
#include <string.h>
#include "mesh.h"
#include "renderer.h"
#include "resource.h"
#include "scanner.h"
#include "parser.h"


/*
=================================================================================================================================================================
Load                                                                                                                                                            =
=================================================================================================================================================================
*/
bool mesh_data_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;


	//** MATERIAL **
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_STRING )
	{
		PARSE_ERR_EXPECTED( "string" );
		return false;
	}
	material_name = new char [strlen(token->value.string)+1];
	strcpy( material_name, token->value.string );

	//** VERTS **
	// verts num
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	vert_coords.resize( token->value.int_ );

	// read the verts
	for( uint i=0; i<vert_coords.size(); i++ )
	{
		if( !ParseArrOfNumbers<float>( scanner, false, true, 3, &vert_coords[i][0] ) ) return false;
	}

	//** FACES **
	// faces num
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	tris.resize( token->value.int_ );
	// read the faces
	for( uint i=0; i<tris.size(); i++ )
	{
		if( !ParseArrOfNumbers<uint>( scanner, false, true, 3, tris[i].vert_ids ) ) return false;
	}

	//** UVS **
	// UVs num
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	tex_coords.resize( token->value.int_ );
	// read the tex_coords
	for( uint i=0; i<tex_coords.size(); i++ )
	{
		if( !ParseArrOfNumbers( scanner, false, true, 2, &tex_coords[i][0] ) ) return false;
	}

	//** VERTEX WEIGHTS **
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	vert_weights.resize( token->value.int_ );
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

		// for all the weights of the current vertes
		for( uint j=0; j<vert_weights[i].bones_num; j++ )
		{
			// read bone id
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
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

	// Sanity checks
	if( vert_coords.size()<1 || tris.size()<1 )
	{
		ERROR( "Vert coords and tris must be filled \"" << filename << "\"" );
		return false;
	}
	if( tex_coords.size()!=0 && tex_coords.size()!=vert_coords.size() )
	{
		ERROR( "Tex coords num must be zero or equal to vertex coords num \"" << filename << "\"" );
		return false;
	}
	if( vert_weights.size()!=0 && vert_weights.size()!=vert_coords.size() )
	{
		ERROR( "Vert weights num must be zero or equal to vertex coords num \"" << filename << "\"" );
		return false;
	}

	CreateAllNormals();
	CreateVertTangents();
	CreateVertIndeces();
	CreateVBOs();

	return true;
}


/*
=================================================================================================================================================================
Unload                                                                                                                                                            =
=================================================================================================================================================================
*/
void mesh_data_t::Unload()
{
	vert_coords.clear();
	vert_normals.clear();
	tris.clear();
	tex_coords.clear();
	vert_tangents.clear();
	vert_indeces.clear();
	if( material_name ) delete [] material_name;
}


/*
=======================================================================================================================================
CreateFaceNormals                                                                                                                     =
=======================================================================================================================================
*/
void mesh_data_t::CreateVertIndeces()
{
	DEBUG_ERR( vert_indeces.size() > 0 );

	vert_indeces.resize( tris.size() * 3 );
	for( uint i=0; i<tris.size(); i++ )
	{
		vert_indeces[i*3+0] = tris[i].vert_ids[0];
		vert_indeces[i*3+1] = tris[i].vert_ids[1];
		vert_indeces[i*3+2] = tris[i].vert_ids[2];
	}
}


/*
=======================================================================================================================================
CreateFaceNormals                                                                                                                     =
=======================================================================================================================================
*/
void mesh_data_t::CreateFaceNormals()
{
	for( uint i=0; i<tris.size(); i++ )
	{
		triangle_t& tri = tris[i];
		const vec3_t& v0 = vert_coords[ tri.vert_ids[0] ];
		const vec3_t& v1 = vert_coords[ tri.vert_ids[1] ];
		const vec3_t& v2 = vert_coords[ tri.vert_ids[2] ];

		tri.normal = ( v1 - v0 ).Cross( v2 - v0 );

		tri.normal.Normalize();
	}
}


/*
=======================================================================================================================================
CreateVertNormals                                                                                                                     =
=======================================================================================================================================
*/
void mesh_data_t::CreateVertNormals()
{
	vert_normals.resize( vert_coords.size() ); // alloc

	for( uint i=0; i<vert_coords.size(); i++ )
		vert_normals[i] = vec3_t( 0.0, 0.0, 0.0 );

	for( uint i=0; i<tris.size(); i++ )
	{
		const triangle_t& tri = tris[i];
		vert_normals[ tri.vert_ids[0] ] += tri.normal;
		vert_normals[ tri.vert_ids[1] ] += tri.normal;
		vert_normals[ tri.vert_ids[2] ] += tri.normal;
	}

	for( uint i=0; i<vert_normals.size(); i++ )
		vert_normals[i].Normalize();
}


/*
=======================================================================================================================================
CreateVertTangents                                                                                                                      =
=======================================================================================================================================
*/
void mesh_data_t::CreateVertTangents()
{
	vert_tangents.resize( vert_coords.size() ); // alloc

	vec_t<vec3_t> bitagents( vert_coords.size() );

	for( uint i=0; i<vert_tangents.size(); i++ )
	{
		vert_tangents[i] = vec4_t( 0.0 );
		bitagents[i] = vec3_t( 0.0 );
	}

	for( uint i=0; i<tris.size(); i++ )
	{
		const triangle_t& tri = tris[i];
		const int i0 = tri.vert_ids[0];
		const int i1 = tri.vert_ids[1];
		const int i2 = tri.vert_ids[2];
		const vec3_t& v0 = vert_coords[ i0 ];
		const vec3_t& v1 = vert_coords[ i1 ];
		const vec3_t& v2 = vert_coords[ i2 ];
		vec3_t edge01 = v1 - v0;
		vec3_t edge02 = v2 - v0;
		vec2_t uvedge01 = tex_coords[i1] - tex_coords[i0];
		vec2_t uvedge02 = tex_coords[i2] - tex_coords[i0];


		float det = (uvedge01.y * uvedge02.x) - (uvedge01.x * uvedge02.y);
		DEBUG_ERR( IsZero(det) );
		det = 1.0f / det;

		vec3_t t = ( edge02 * uvedge01.y - edge01 * uvedge02.y ) * det;
		vec3_t b = ( edge02 * uvedge01.x - edge01 * uvedge02.x ) * det;
		t.Normalize();
		b.Normalize();

		vert_tangents[i0] += vec4_t(t, 1.0);
		vert_tangents[i1] += vec4_t(t, 1.0);
		vert_tangents[i2] += vec4_t(t, 1.0);

		bitagents[i0] += b;
		bitagents[i1] += b;
		bitagents[i2] += b;
	}

	for( uint i=0; i<vert_tangents.size(); i++ )
	{
		vec3_t t = vec3_t(vert_tangents[i]);
		const vec3_t& n = vert_normals[i];
		vec3_t& b = bitagents[i];

		//t = t - n * n.Dot(t);
		t.Normalize();

		b.Normalize();

		float w = ( (n.Cross(t)).Dot( b ) < 0.0) ? 1.0 : -1.0;

		vert_tangents[i] = vec4_t( t, w );
	}

	bitagents.clear();

//	for( uint i=0; i<tris.size(); i++ )
//	{
//		const triangle_t& tri = tris[i];
//		const int vi1 = tri.vert_ids[0];
//		const int vi2 = tri.vert_ids[1];
//		const int vi3 = tri.vert_ids[2];
//		vec3_t v1 = verts[ vi3 ].coords - verts[ vi1 ].coords;
//		vec3_t v2 = verts[ vi2 ].coords - verts[ vi1 ].coords;
//
//		vec2_t st1 = uvs[vi3] - uvs[vi1];
//		vec2_t st2 = uvs[vi2] - uvs[vi1];
//
//
//		float coef = 1 / (st1.x * st2.y - st2.x * st1.y);
//		vec3_t tangent;
//
//		tangent.x = coef * ((v1.x * st2.y)  + (v2.x * -st1.y));
//		tangent.y = coef * ((v1.y * st2.y)  + (v2.y * -st1.y));
//		tangent.z = coef * ((v1.z * st2.y)  + (v2.z * -st1.y));
//
//
//		face_tangents[i] = tangent;
//	}

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
=======================================================================================================================================
CreateVBOs                                                                                                                            =
=======================================================================================================================================
*/
void mesh_data_t::CreateVBOs()
{
	vbos.vert_indeces.Create( GL_ELEMENT_ARRAY_BUFFER, vert_indeces.GetSizeInBytes(), &vert_indeces[0], GL_STATIC_DRAW );
	vbos.vert_coords.Create( GL_ARRAY_BUFFER, vert_coords.GetSizeInBytes(), &vert_coords[0], GL_STATIC_DRAW );
	vbos.vert_normals.Create( GL_ARRAY_BUFFER, vert_normals.GetSizeInBytes(), &vert_normals[0], GL_STATIC_DRAW );
	vbos.vert_tangents.Create( GL_ARRAY_BUFFER, vert_tangents.GetSizeInBytes(), &vert_tangents[0], GL_STATIC_DRAW );
	if( tex_coords.size() > 1 )
		vbos.tex_coords.Create( GL_ARRAY_BUFFER, tex_coords.GetSizeInBytes(), &tex_coords[0], GL_STATIC_DRAW );
	if( vert_weights.size() > 1 )
		vbos.vert_weights.Create( GL_ARRAY_BUFFER, vert_weights.GetSizeInBytes(), &vert_weights[0], GL_STATIC_DRAW );

}


/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool mesh_t::Load( const char* filename )
{
	mesh_data = rsrc::mesh_datas.Load( filename );
	if( !mesh_data ) return false;

	material = rsrc::materials.Load( mesh_data->material_name );
	if( !material ) return false;

	return true;
}


/*
=======================================================================================================================================
Unload                                                                                                                                =
=======================================================================================================================================
*/
void mesh_t::Unload()
{
	rsrc::mesh_datas.Unload( mesh_data );
	rsrc::materials.Unload( material );
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void mesh_t::Render()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );


	if( material->attribute_locs.position != -1 )
	{
		mesh_data->vbos.vert_coords.Bind();
		glVertexAttribPointer( material->attribute_locs.position, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attribute_locs.position );
	}

	if( material->attribute_locs.normal != -1 )
	{
		mesh_data->vbos.vert_normals.Bind();
		glVertexAttribPointer( material->attribute_locs.normal, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attribute_locs.normal );
	}

	if( material->attribute_locs.tex_coords != -1 )
	{
		mesh_data->vbos.tex_coords.Bind();
		glVertexAttribPointer( material->attribute_locs.tex_coords, 2, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attribute_locs.tex_coords );
	}

	if( material->attribute_locs.tanget != -1 )
	{
		mesh_data->vbos.vert_tangents.Bind();
		glVertexAttribPointer( material->attribute_locs.tanget, 4, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attribute_locs.tanget );
	}

	mesh_data->vbos.vert_indeces.Bind();

	glDrawElements( GL_TRIANGLES, mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );


	if( material->attribute_locs.position != -1 ) glDisableVertexAttribArray( material->attribute_locs.position );
	if( material->attribute_locs.normal != -1 ) glDisableVertexAttribArray( material->attribute_locs.normal );
	if( material->attribute_locs.tex_coords != -1 ) glDisableVertexAttribArray( material->attribute_locs.tex_coords );
	if( material->attribute_locs.tanget != -1 ) glDisableVertexAttribArray( material->attribute_locs.tanget );


	vbo_t::UnbindAllTargets();
	glPopMatrix();
}


/*
=======================================================================================================================================
RenderNormals                                                                                                                         =
=======================================================================================================================================
*/
void mesh_t::RenderNormals()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	//glLineWidth( 1.0 );
	glColor3f( 0.0, 0.0, 1.0 );

	glBegin( GL_LINES );
		for( uint i=0; i<mesh_data->tris.size(); i++ )
		{
			triangle_t* tri = &mesh_data->tris[i];
			for( int j=0; j<3; j++ )
			{
				const vec3_t& normal = mesh_data->vert_normals[ tri->vert_ids[j] ];
				const vec3_t& coord = mesh_data->vert_coords[ tri->vert_ids[j] ];

				vec3_t vec0;
				vec0 = (normal * 0.1f) + coord;

				glVertex3fv( &(const_cast<vec3_t&>(coord))[0] );
				glVertex3fv( &vec0[0] );
			}
		}
	glEnd();

	glPopMatrix();
}


/*
=======================================================================================================================================
RenderTangents                                                                                                                        =
=======================================================================================================================================
*/
void mesh_t::RenderTangents()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	//glLineWidth( 1.0 );
	glColor3f( 1.0, 0.0, 0.0 );

	glBegin( GL_LINES );
		for( uint i=0; i<mesh_data->vert_coords.size(); i++ )
		{
				vec3_t vec0 = (vec3_t(mesh_data->vert_tangents[i]) * 0.1f) + mesh_data->vert_coords[i];

				glVertex3fv( &mesh_data->vert_coords[i][0] );
				glVertex3fv( &vec0[0] );
		}
	glEnd();

	glPopMatrix();
}



/*
=======================================================================================================================================
RenderDepth                                                                                                                           =
=======================================================================================================================================
*/
void mesh_t::RenderDepth()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	/*glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, &mesh_data->vert_coords[0][0] );

	if( material->grass_map )
	{
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer( 2, GL_FLOAT, 0, &mesh_data->uvs[0] );
	}

	glDrawElements( GL_TRIANGLES, mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, &mesh_data->vert_indeces[0] );

	glDisableClientState( GL_VERTEX_ARRAY );
	if( material->grass_map )
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );*/

	mesh_data->vbos.vert_coords.Bind();
	glVertexPointer( 3, GL_FLOAT, 0, NULL );

	if( material->grass_map )
	{
		mesh_data->vbos.tex_coords.Bind();
		glTexCoordPointer( 2, GL_FLOAT, 0, NULL );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	mesh_data->vbos.vert_indeces.Bind();

	glEnableClientState( GL_VERTEX_ARRAY );

	glDrawElements( GL_TRIANGLES, mesh_data->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	vbo_t::UnbindAllTargets();

	glPopMatrix();
}
