// XXX

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

void main()
{
	Flare flare = flares[gl_InstanceID];

	vTexCoords = vec3((position * 0.5) + 0.5, flare.alphaDepth.y);

	vec4 posScale = flare.posScale;
	gl_Position = vec4(position * posScale.zw + posScale.xy , 0.0, 1.0);
}

#pragma anki start fragmentShader

uniform sampler2DArray images;

in vec3 vTexCoords;

out vec3 fColor;

void main()
{
	fColor = texture(images, vTexCoords).rgb;
}
