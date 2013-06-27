// XXX

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#define MAX_GHOSTS 4
#define FMAX_GHOSTS (float(MAX_GHOSTS))
#define GHOST_DISPERSAL (0.7)
#define HALO_WIDTH 0.4
#define DISTORTION 4.0

uniform sampler2D tex;
uniform sampler2D lensDirtTex;

in vec2 vTexCoords;

out vec3 fColor;

vec3 textureDistorted(
	in sampler2D tex,
	in vec2 texcoord,
	in vec2 direction, // direction of distortion
	in vec3 distortion ) // per-channel distortion factor  
{
	return vec3(
		texture(tex, texcoord + direction * distortion.r).r,
		texture(tex, texcoord + direction * distortion.g).g,
		texture(tex, texcoord + direction * distortion.b).b);
}

void main()
{
	vec2 texcoord = -vTexCoords + vec2(1.0);

	vec2 imgSize = vec2(textureSize(tex, 0));

	vec2 ghostVec = (vec2(0.5) - texcoord) * GHOST_DISPERSAL;

	const vec2 texelSize = 1.0 / vec2(TEX_DIMENSIONS);

	const vec3 distortion = 
		vec3(-texelSize.x * DISTORTION, 0.0, texelSize.x * DISTORTION);

	// sample ghosts:  
	vec3 result = vec3(0.0);
	for(int i = 0; i < MAX_GHOSTS; ++i) 
	{ 
		vec2 offset = fract(texcoord + ghostVec * float(i));

		float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
		weight = pow(1.0 - weight, 10.0);

		result += textureDistorted(tex, offset, offset, distortion) * weight;
	}

	// sample halo:
	vec2 haloVec = normalize(ghostVec) * HALO_WIDTH;
	float weight = 
		length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));
	weight = pow(1.0 - weight, 5.0);
	result += textureDistorted(tex, texcoord + haloVec, haloVec, distortion) 
		* weight;

	// lens dirt
	result *= texture(lensDirtTex, vTexCoords).rgb;

	// Write
	fColor = result;
}
