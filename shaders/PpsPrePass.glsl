#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D isFai;
uniform sampler2D ppsSsaoFai;

in vec2 vTexCoords;

layout(location = 0) out vec3 fFragColor;


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main(void)
{
	fFragColor = texture2D(isFai, vTexCoords).rgb;

	#if defined(SSAO_ENABLED)
		float ssaoFactor = texture2D(ppsSsaoFai, vTexCoords).r;
		fFragColor *= ssaoFactor;
	#endif
}

