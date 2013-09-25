// Draw sprite lens flare

#pragma anki start vertexShader

// Per flare information
struct Flare
{
	vec4 posScale; // xy: Position, z: Scale, w: Scale again
	vec4 alphaDepth; // x: alpha, y: texture depth
};

// The block contains data for all flares
layout(std140) uniform flaresBlock
{
	Flare flares[MAX_FLARES];
};

layout(location = 0) in vec2 position;

out vec3 vTexCoords;
flat out float vAlpha;

void main()
{
	Flare flare = flares[gl_InstanceID];

	vTexCoords = vec3((position * 0.5) + 0.5, flare.alphaDepth.y);

	vec4 posScale = flare.posScale;
	gl_Position = vec4(position * posScale.zw + posScale.xy , 0.0, 1.0);

	vAlpha = flare.alphaDepth.x;
}

#pragma anki start fragmentShader
#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/Common.glsl"

#define SINGLE_FLARE 0

#if SINGLE_FLARE
uniform sampler2D image;
#else
uniform DEFAULT_FLOAT_PRECISION sampler2DArray images;
#endif

in vec3 vTexCoords;
flat in float vAlpha;

out vec3 fColor;

void main()
{
#if SINGLE_FLARE
	vec3 col = texture(images, vTexCoords.st).rgb;
#else
	vec3 col = texture(images, vTexCoords).rgb;
#endif

	fColor = col * vAlpha;
}
