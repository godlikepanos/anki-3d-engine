// XXX

#pragma anki start vertexShader

// Per flare information
struct Flare
{
	vec4 posAndScale; // xy: Position, z: Scale, w: Scale again
};

// The block contains data for all flares
layout(std140) uniform FlaresBlock
{
	Flare flares[MAX_FLARES];
};

layout(location = 0) in vec2 position;

out vec3 vTexCoords;

void main()
{
	vTexCoords = vec3((position * 0.5) + 0.5, float(gl_InstanceID));

	vec4 posAndScale = flares[gl_InstanceID].posAndScale;
	gl_Position = vec4(position * posAndScale.zw + posAndScale.xy , 0.0, 1.0);
}

#pragma anki start fragmentShader

uniform sampler2DArray images;

in vec3 vTexCoords;

out vec3 fColor;

void main()
{
	fColor = texture(images, vec3(vTexCoords.st, 0.0)).rgb;
}
