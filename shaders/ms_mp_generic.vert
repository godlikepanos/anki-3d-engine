/**
This a generic shader to fill the deferred shading buffers. You can always build your own if you dont need to write in all the
buffers.
*/
varying vec3 normal;


#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ ) || defined( _HAS_SPECULAR_MAP_ )
	#define NEEDS_TEX_MAPPING 1
#else
	#define NEEDS_TEX_MAPPING 0
#endif


#if defined( _HAS_NORMAL_MAP_ ) || defined( _PARALLAX_MAPPING_ )
	#define NEEDS_TANGENT 1
#else
	#define NEEDS_TANGENT 0
#endif


varying vec2 tex_coords_v2f;
uniform vec2 tex_coords;

attribute vec4 tangent;
varying vec3 tangent_v2f;
varying float w_v2f;

varying vec3 eye_v2f; ///< In tangent space
varying vec3 vert_pos_eye_space_v2f;



//=====================================================================================================================================
// main                                                                                                                               =
//=====================================================================================================================================
void main()
{
	// calculate the vert pos, normal and tangent

	// if we have hardware skinning then:
	#if defined( _HARDWARE_SKINNING_ )
		mat3 _rot;
		vec3 _tsl;

		HWSkinning( _rot, _tsl );

		normal = gl_NormalMatrix * ( _rot * gl_Normal );

		#if NEEDS_TANGENT
			tangent_v2f = gl_NormalMatrix * ( _rot * vec3(tangent) );
		#endif

		vec3 pos_lspace = ( _rot * gl_Vertex.xyz) + _tsl;
		gl_Position =  gl_ModelViewProjectionMatrix * vec4(pos_lspace, 1.0);

	// if DONT have hardware skinning
	#else
		normal = gl_NormalMatrix * gl_Normal;

		#if NEEDS_TANGENT
			tangent_v2f = gl_NormalMatrix * vec3(tangent);
		#endif

		gl_Position = ftransform();
	#endif


	// calculate the rest

	#if NEEDS_TEX_MAPPING
		tex_coords_v2f = gl_MultiTexCoord0.xy;
	#endif


	#if NEEDS_TANGENT
		w_v2f = tangent.w;
	#endif


	#if defined( _ENVIRONMENT_MAPPING_ )
		vert_pos_eye_space_v2f = vec3( gl_ModelViewMatrix * gl_Vertex );
	#endif


	#if defined( _PARALLAX_MAPPING_ )
		vec3 t = gl_NormalMatrix * tangent.xyz;
		vec3 n = normal;
		vec3 b = cross( n, t ) /* tangent.w*/;

		vec3 eye_pos = gl_Position.xyz - gl_ModelViewMatrixInverse[3].xyz;
		eye_pos = eye_pos - ( gl_ModelViewMatrixInverse * gl_Vertex ).xyz;

		mat3 tbn_mat = mat3( t, b, n );
		eye = tbn_mat * eye_pos;
		//eye.y = -eye.y;
		//eye.x = -eye.x;
	#endif
}

