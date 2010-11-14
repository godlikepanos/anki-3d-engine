/// Control defines:
/// ALPHA_TESTING, HARDWARE_SKINNING

#pragma anki vertShaderBegins

#pragma anki include "shaders/hw_skinning.glsl"

uniform mat4 modelViewProjectionMat;

in vec3 position;

#if defined(ALPHA_TESTING)
	in vec2 texCoords;
	out vec2 vTexCoords;
#endif

void main()
{
	#if defined(ALPHA_TESTING)
		vTexCoords = texCoords;
	#endif

	#if defined(HARDWARE_SKINNING)
		mat3 _rot_;
		vec3 _tsl_;

		HWSkinning(_rot_, _tsl_);

		vec3 _posLocalSpace_ = (_rot_ * position) + _tsl_;
		gl_Position = modelViewProjectionMat * vec4(_posLocalSpace_, 1.0);
	#else
	  gl_Position = modelViewProjectionMat * vec4(position, 1.0);
	#endif
}

#pragma anki fragShaderBegins

#if defined(ALPHA_TESTING)
	uniform sampler2D diffuseMap;
	in vec2 vTexCoords;
#endif

void main()
{
	#if defined(ALPHA_TESTING)
		if(texture2D(diffuseMap, vTexCoords).a == 0.0)
		{
			discard;
		}
	#endif
}
