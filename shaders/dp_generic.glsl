#pragma anki vert_shader_begins

#pragma anki include "shaders/hw_skinning.glsl"

varying vec2 tex_coords_v2f;

void main()
{
	#if defined( _GRASS_LIKE_ )
		tex_coords_v2f = gl_MultiTexCoord0.xy;
	#endif

	#if defined( _HW_SKINNING_ )
		mat3 _rot;
		vec3 _tsl;

		HWSkinning( _rot, _tsl );
		
		vec3 pos_lspace = ( _rot * gl_Vertex.xyz) + _tsl;
		gl_Position =  gl_ModelViewProjectionMatrix * vec4(pos_lspace, 1.0);
	#else
	  gl_Position = ftransform();
	#endif
}

#pragma anki frag_shader_begins

uniform sampler2D diffuse_map;
varying vec2 tex_coords_v2f;

void main()
{
	#if defined( _GRASS_LIKE_ )
		vec4 _diff = texture2D( diffuse_map, tex_coords_v2f );
		if( _diff.a == 0.0 ) discard;
	#endif
}
