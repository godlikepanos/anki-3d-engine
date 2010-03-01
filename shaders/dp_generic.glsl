//
#pragma anki vertShaderBegins

#pragma anki include "shaders/hw_skinning.glsl"

attribute vec3 position;
attribute vec2 texCoords;

varying vec2 texCoords_v2f;

void main()
{
	#if defined( _GRASS_LIKE_ )
		texCoords_v2f = texCoords;
	#endif

	#if defined( _HW_SKINNING_ )
		mat3 _rot;
		vec3 _tsl;

		HWSkinning( _rot, _tsl );
		
		vec3 pos_lspace = ( _rot * position) + _tsl;
		gl_Position =  gl_ModelViewProjectionMatrix * vec4(pos_lspace, 1.0);
	#else
	  gl_Position =  gl_ModelViewProjectionMatrix * vec4(position, 1.0);
	#endif
}

#pragma anki fragShaderBegins

uniform sampler2D diffuseMap;
varying vec2 texCoords_v2f;

void main()
{
	#if defined( _GRASS_LIKE_ )
		vec4 _diff = texture2D( diffuseMap, texCoords_v2f );
		if( _diff.a == 0.0 ) discard;
	#endif
}
