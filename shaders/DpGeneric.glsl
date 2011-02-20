/// Control defines:
/// ALPHA_TESTING

#pragma anki vertShaderBegins

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

	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
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
