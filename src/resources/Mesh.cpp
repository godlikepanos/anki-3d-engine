#include "Mesh.h"
#include "renderer.h"
#include "Resource.h"
#include "Scanner.h"
#include "parser.h"


//=====================================================================================================================================
// load                                                                                                                               =
//=====================================================================================================================================
bool Mesh::load( const char* filename )
{
	Scanner scanner;
	if( !scanner.loadFile( filename ) ) return false;

	const Scanner::Token* token;


	//** MATERIAL **
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_STRING )
	{
		PARSE_ERR_EXPECTED( "string" );
		return false;
	}
	materialName = token->value.string;

	//** DP_MATERIAL **
	/*token = &scanner.getNextToken();
	if( token->code != Scanner::TC_STRING )
	{
		PARSE_ERR_EXPECTED( "string" );
		return false;
	}
	dp_materialName = token->value.string;*/

	//** VERTS **
	// verts num
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	vertCoords.resize( token->value.int_ );

	// read the verts
	for( uint i=0; i<vertCoords.size(); i++ )
	{
		if( !ParseArrOfNumbers<float>( scanner, false, true, 3, &vertCoords[i][0] ) ) return false;
	}

	//** FACES **
	// faces num
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	tris.resize( token->value.int_ );
	// read the faces
	for( uint i=0; i<tris.size(); i++ )
	{
		if( !ParseArrOfNumbers<uint>( scanner, false, true, 3, tris[i].vertIds ) ) return false;
	}

	//** UVS **
	// UVs num
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	texCoords.resize( token->value.int_ );
	// read the texCoords
	for( uint i=0; i<texCoords.size(); i++ )
	{
		if( !ParseArrOfNumbers( scanner, false, true, 2, &texCoords[i][0] ) ) return false;
	}

	//** VERTEX WEIGHTS **
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	vertWeights.resize( token->value.int_ );
	for( uint i=0; i<vertWeights.size(); i++ )
	{
		// get the bone connections num
		token = &scanner.getNextToken();
		if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
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
		if( token->value.int_ > VertexWeight::MAX_BONES_PER_VERT )
		{
			ERROR( "Cannot have more than " << VertexWeight::MAX_BONES_PER_VERT << " bones per vertex" );
			return false;
		}
		vertWeights[i].bonesNum = token->value.int_;

		// for all the weights of the current vertes
		for( uint j=0; j<vertWeights[i].bonesNum; j++ )
		{
			// read bone id
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			vertWeights[i].boneIds[j] = token->value.int_;

			// read the weight of that bone
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_FLOAT )
			{
				PARSE_ERR_EXPECTED( "float" );
				return false;
			}
			vertWeights[i].weights[j] = token->value.float_;
		}
	}

	// Sanity checks
	if( vertCoords.size()<1 || tris.size()<1 )
	{
		ERROR( "Vert coords and tris must be filled \"" << filename << "\"" );
		return false;
	}
	if( texCoords.size()!=0 && texCoords.size()!=vertCoords.size() )
	{
		ERROR( "Tex coords num must be zero or equal to vertex coords num \"" << filename << "\"" );
		return false;
	}
	if( vertWeights.size()!=0 && vertWeights.size()!=vertCoords.size() )
	{
		ERROR( "Vert weights num must be zero or equal to vertex coords num \"" << filename << "\"" );
		return false;
	}

	createAllNormals();
	if( texCoords.size() > 0 ) createVertTangents();
	createVertIndeces();
	createVBOs();
	calcBSphere();

	return true;
}


//=====================================================================================================================================
// unload                                                                                                                             =
//=====================================================================================================================================
void Mesh::unload()
{
	// ToDo: add when finalized
}


//=====================================================================================================================================
// createFaceNormals                                                                                                                  =
//=====================================================================================================================================
void Mesh::createVertIndeces()
{
	DEBUG_ERR( vertIndeces.size() > 0 );

	vertIndeces.resize( tris.size() * 3 );
	for( uint i=0; i<tris.size(); i++ )
	{
		vertIndeces[i*3+0] = tris[i].vertIds[0];
		vertIndeces[i*3+1] = tris[i].vertIds[1];
		vertIndeces[i*3+2] = tris[i].vertIds[2];
	}
}


//=====================================================================================================================================
// createFaceNormals                                                                                                                  =
//=====================================================================================================================================
void Mesh::createFaceNormals()
{
	for( uint i=0; i<tris.size(); i++ )
	{
		Triangle& tri = tris[i];
		const vec3_t& v0 = vertCoords[ tri.vertIds[0] ];
		const vec3_t& v1 = vertCoords[ tri.vertIds[1] ];
		const vec3_t& v2 = vertCoords[ tri.vertIds[2] ];

		tri.normal = ( v1 - v0 ).Cross( v2 - v0 );

		tri.normal.Normalize();
	}
}


//=====================================================================================================================================
// createVertNormals                                                                                                                  =
//=====================================================================================================================================
void Mesh::createVertNormals()
{
	vertNormals.resize( vertCoords.size() ); // alloc

	for( uint i=0; i<vertCoords.size(); i++ )
		vertNormals[i] = vec3_t( 0.0, 0.0, 0.0 );

	for( uint i=0; i<tris.size(); i++ )
	{
		const Triangle& tri = tris[i];
		vertNormals[ tri.vertIds[0] ] += tri.normal;
		vertNormals[ tri.vertIds[1] ] += tri.normal;
		vertNormals[ tri.vertIds[2] ] += tri.normal;
	}

	for( uint i=0; i<vertNormals.size(); i++ )
		vertNormals[i].Normalize();
}


//=====================================================================================================================================
// createVertTangents                                                                                                                 =
//=====================================================================================================================================
void Mesh::createVertTangents()
{
	vertTangents.resize( vertCoords.size() ); // alloc

	Vec<vec3_t> bitagents( vertCoords.size() );

	for( uint i=0; i<vertTangents.size(); i++ )
	{
		vertTangents[i] = vec4_t( 0.0 );
		bitagents[i] = vec3_t( 0.0 );
	}

	for( uint i=0; i<tris.size(); i++ )
	{
		const Triangle& tri = tris[i];
		const int i0 = tri.vertIds[0];
		const int i1 = tri.vertIds[1];
		const int i2 = tri.vertIds[2];
		const vec3_t& v0 = vertCoords[ i0 ];
		const vec3_t& v1 = vertCoords[ i1 ];
		const vec3_t& v2 = vertCoords[ i2 ];
		vec3_t edge01 = v1 - v0;
		vec3_t edge02 = v2 - v0;
		vec2_t uvedge01 = texCoords[i1] - texCoords[i0];
		vec2_t uvedge02 = texCoords[i2] - texCoords[i0];


		float det = (uvedge01.y * uvedge02.x) - (uvedge01.x * uvedge02.y);
		DEBUG_ERR( IsZero(det) );
		det = 1.0f / det;

		vec3_t t = ( edge02 * uvedge01.y - edge01 * uvedge02.y ) * det;
		vec3_t b = ( edge02 * uvedge01.x - edge01 * uvedge02.x ) * det;
		t.Normalize();
		b.Normalize();

		vertTangents[i0] += vec4_t(t, 1.0);
		vertTangents[i1] += vec4_t(t, 1.0);
		vertTangents[i2] += vec4_t(t, 1.0);

		bitagents[i0] += b;
		bitagents[i1] += b;
		bitagents[i2] += b;
	}

	for( uint i=0; i<vertTangents.size(); i++ )
	{
		vec3_t t = vec3_t(vertTangents[i]);
		const vec3_t& n = vertNormals[i];
		vec3_t& b = bitagents[i];

		//t = t - n * n.Dot(t);
		t.Normalize();

		b.Normalize();

		float w = ( (n.Cross(t)).Dot( b ) < 0.0) ? 1.0 : -1.0;

		vertTangents[i] = vec4_t( t, w );
	}

	bitagents.clear();
}


//=====================================================================================================================================
// createVBOs                                                                                                                         =
//=====================================================================================================================================
void Mesh::createVBOs()
{
	vbos.vertIndeces.Create( GL_ELEMENT_ARRAY_BUFFER, vertIndeces.GetSizeInBytes(), &vertIndeces[0], GL_STATIC_DRAW );
	vbos.vertCoords.Create( GL_ARRAY_BUFFER, vertCoords.GetSizeInBytes(), &vertCoords[0], GL_STATIC_DRAW );
	vbos.vertNormals.Create( GL_ARRAY_BUFFER, vertNormals.GetSizeInBytes(), &vertNormals[0], GL_STATIC_DRAW );
	if( vertTangents.size() > 1 )
		vbos.vertTangents.Create( GL_ARRAY_BUFFER, vertTangents.GetSizeInBytes(), &vertTangents[0], GL_STATIC_DRAW );
	if( texCoords.size() > 1 )
		vbos.texCoords.Create( GL_ARRAY_BUFFER, texCoords.GetSizeInBytes(), &texCoords[0], GL_STATIC_DRAW );
	if( vertWeights.size() > 1 )
		vbos.vertWeights.Create( GL_ARRAY_BUFFER, vertWeights.GetSizeInBytes(), &vertWeights[0], GL_STATIC_DRAW );
}


//=====================================================================================================================================
// calcBSphere                                                                                                                        =
//=====================================================================================================================================
void Mesh::calcBSphere()
{
	bsphere.Set( &vertCoords[0], 0, vertCoords.size() );
}
