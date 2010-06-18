#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D isFai;
uniform sampler2D ppsSsaoFai;

varying vec2 texCoords;


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main(void)
{
	vec3 color = texture2D(isFai, texCoords).rgb;

	#if defined(SSAO_ENABLED)
		float ssaoFactor = texture2D(ppsSsaoFai, texCoords).a;
		color *= ssaoFactor;
	#endif

	gl_FragData[0].rgb = color;
}

