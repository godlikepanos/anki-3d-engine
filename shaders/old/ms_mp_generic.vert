/*
This a generic shader to fill the deferred shading buffers. You can always build your own if you dont need to write in all the
buffers.
*/
varying vec3 normal;

#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ ) || defined( _HAS_SPECULAR_MAP_ )
varying vec2 tex_coords;
#endif


#if defined( _HAS_NORMAL_MAP_ ) || defined( _PARALLAX_MAPPING_ )
attribute vec4 tangent;
#endif

#if defined( _HAS_NORMAL_MAP_ )
varying vec3 tangent_v;
varying float w;
#endif

#if defined( _PARALLAX_MAPPING_ )
varying vec3 eye; // in tangent space
#endif


void main()
{
	normal = gl_NormalMatrix * gl_Normal;
	gl_Position = ftransform();

#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ ) || defined( _HAS_SPECULAR_MAP_ )
	tex_coords = gl_MultiTexCoord0.xy;
#else
	gl_FrontColor = gl_Color;
#endif

#if defined( _HAS_NORMAL_MAP_ )
	tangent_v = gl_NormalMatrix * vec3(tangent);
	w = tangent.w;
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

