/// Control defines:
/// ALPHA_TESTING

#pragma anki start vertexShader

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

#pragma anki start fragmentShader

#if defined(ALPHA_TESTING)
	uniform sampler2D diffuseMap;
	uniform float alphaTestingTolerance = 0.5; ///< Below this value the pixels are getting discarded 
	in vec2 vTexCoords;
#endif

void main()
{
#if defined(ALPHA_TESTING)
	if(texture2D(diffuseMap, vTexCoords).a < alphaTestingTolerance)
	{
		discard;
	}
#endif
}
