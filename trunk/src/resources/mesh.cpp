#include "mesh.h"
#include "renderer.h"
#include "resource.h"
#include "scanner.h"
#include "parser.h"


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool mesh_t::Load( const char* filename )
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
	material_name = token->value.string;

	//** DP_MATERIAL **
	/*token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_STRING )
	{
		PARSE_ERR_EXPECTED( "string" );
		return false;
	}
	dp_material_name = token->value.string;*/

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
	CalcBSphere();

	return true;
}


//=====================================================================================================================================
// Unload                                                                                                                             =
//=====================================================================================================================================
void mesh_t::Unload()
{
	// ToDo: add when finalized
}


//=====================================================================================================================================
// CreateFaceNormals                                                                                                                  =
//=====================================================================================================================================
void mesh_t::CreateVertIndeces()
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


//=====================================================================================================================================
// CreateFaceNormals                                                                                                                  =
//=====================================================================================================================================
void mesh_t::CreateFaceNormals()
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


//=====================================================================================================================================
// CreateVertNormals                                                                                                                  =
//=====================================================================================================================================
void mesh_t::CreateVertNormals()
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


//=====================================================================================================================================
// CreateVertTangents                                                                                                                 =
//=====================================================================================================================================
void mesh_t::CreateVertTangents()
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
}


//=====================================================================================================================================
// CreateVBOs                                                                                                                         =
//=====================================================================================================================================
void mesh_t::CreateVBOs()
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


//=====================================================================================================================================
// CalcBSphere                                                                                                                        =
//=====================================================================================================================================
void mesh_t::CalcBSphere()
{
	bsphere.Set( &vert_coords[0], 0, vert_coords.size() );
}
