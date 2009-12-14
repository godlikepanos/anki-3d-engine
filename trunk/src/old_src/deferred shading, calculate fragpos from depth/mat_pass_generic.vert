/*
This a generic shader to fill the deferred shading buffers. You can always build your own if you dont need to write in all the
buffers.
*/
varying vec3 normal;

#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ )
varying vec2 txtr_coords;
#endif

#if defined( _HAS_NORMAL_MAP_ )
attribute vec3 tangent;
varying vec3 tangent_v;
#endif

void main()
{
	normal = gl_NormalMatrix * gl_Normal;

#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ )
	txtr_coords = gl_MultiTexCoord0.xy;
#else
	gl_FrontColor = gl_Color;
#endif

#if defined( _HAS_NORMAL_MAP_ )
	tangent_v = tangent;
#endif

	gl_Position = ftransform();
}

