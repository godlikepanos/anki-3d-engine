/// @file
///
/// Illumination stage lighting pass shader program

#pragma anki start vertexShader

#if !TILES_X_COUNT || !TILES_Y_COUNT
#	error See file
#endif

layout(location = 0) in vec2 position;

uniform vec4 limitsOfNearPlane;

out vec2 vTexCoords;
flat out int vInstanceId;
out vec2 vLimitsOfNearPlaneOpt;

void main()
{
	vec2 ij = vec2(
		float(gl_InstanceID % TILES_X_COUNT), 
		float(gl_InstanceID / TILES_X_COUNT));

	vInstanceId = int(gl_InstanceID);

	const vec2 SIZES = 
		vec2(1.0 / float(TILES_X_COUNT), 1.0 / float(TILES_Y_COUNT));

	vTexCoords = (position + ij) * SIZES;
	vec2 vertPosNdc = vTexCoords * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);

	vLimitsOfNearPlaneOpt = 
		(vTexCoords * limitsOfNearPlane.zw) - limitsOfNearPlane.xy;
}

#pragma anki start fragmentShader
#pragma anki include "shaders/CommonFrag.glsl"
#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/LinearDepth.glsl"
#pragma anki include "shaders/IsCommon.glsl"

#define ATTENUATION_FINE 0

#define ATTENUATION_BOOST (0.05)

/// @name Uniforms
/// @{

// Common uniforms between lights
layout(std140, row_major) uniform commonBlock
{
	/// Packs:
	/// - xy: Planes. For the calculation of frag pos in view space
	uniform vec4 planes_;

	uniform vec4 sceneAmbientColor;

	uniform vec4 groundLightDir;
};

#define planes planes_.xy

layout(std140) uniform pointLightsBlock
{
	Light pointLights[MAX_POINT_LIGHTS];
};

layout(std140) uniform spotLightsBlock
{
	SpotLight spotLights[MAX_SPOT_LIGHTS];
};

layout(std140) uniform spotTexLightsBlock
{
	SpotTexLight spotTexLights[MAX_SPOT_TEX_LIGHTS];
};

#if __VERSION__ > 430
layout(std430) 
#else
layout(std140) 
#endif
	uniform tilesBlock
{
	Tile tiles[TILES_COUNT];
};

uniform highp usampler2D msFai0;
uniform sampler2D msDepthFai;

//uniform sampler2D lightTextures[MAX_SPOT_LIGHTS];
uniform mediump sampler2DArrayShadow shadowMapArr;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
flat in int vInstanceId;
in vec2 vLimitsOfNearPlaneOpt;
/// @}

/// @name Output
/// @{
out vec3 fColor;
/// @}

//==============================================================================
/// @return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = texture(msDepthFai, vTexCoords).r;

	vec3 fragPosVspace;
	fragPosVspace.z = planes.y / (planes.x + depth);
	fragPosVspace.xy = vLimitsOfNearPlaneOpt * fragPosVspace.z;
	fragPosVspace.z = -fragPosVspace.z;

	return fragPosVspace;
}

//==============================================================================
float calcAttenuationFactor(
	in vec3 fragPosVspace, 
	in Light light,
	out vec3 rayDir)
{
	// get the vector from the frag to the light
	vec3 frag2LightVec = light.posRadius.xyz - fragPosVspace;

	// Instead of using normalize(frag2LightVec) we brake the operation 
	// because we want fragLightDist for the calc of the attenuation
	float fragLightDistSquared = dot(frag2LightVec, frag2LightVec);
	float fragLightDist = sqrt(fragLightDistSquared);
	rayDir = frag2LightVec / fragLightDist;

	float att = max((1.0 + ATTENUATION_BOOST) 
		- fragLightDist / light.posRadius.w, 0.0);

	return att;
}

//==============================================================================
float calcLambertTerm(in vec3 normal, in vec3 rayDir)
{
	return max(0.0, dot(normal, rayDir));
}

//==============================================================================
/// Performs phong lighting using the MS FAIs and a few other things
vec3 calcPhong(in vec3 fragPosVspace, in vec3 diffuse, 
	in vec2 specularAll, in vec3 normal, in Light light, in vec3 rayDir)
{
	// Diffuse
	vec3 difCol = diffuse * light.diffuseColorShadowmapId.rgb;

	// Specular
	vec3 eyeVec = normalize(fragPosVspace);
	vec3 h = normalize(rayDir - eyeVec);
	float specIntensity = pow(max(0.0, dot(normal, h)), specularAll.g);
	vec3 specCol = 
		light.specularColorTexId.rgb * (specIntensity * specularAll.r);
	
	// end
	return difCol + specCol;
}

//==============================================================================
float calcSpotFactor(in SpotLight light, in vec3 fragPosVspace)
{
	vec3 l = normalize(fragPosVspace - light.lightBase.posRadius.xyz);

	float costheta = dot(l, light.lightDir.xyz);
	float spotFactor = smoothstep(
		light.outerCosInnerCos.x, 
		light.outerCosInnerCos.y, 
		costheta);

	return spotFactor;
}

//==============================================================================
#if PCF == 1
float pcfLow(in sampler2DArrayShadow shadowMap, in vec3 shadowUv)
{
	float shadowCol = textureOffset(shadowMap, shadowUv, ivec2(-1, -1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(0, -1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(1, -1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(-1, 0));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(0, 0));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(1, 0));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(-1, 1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(0, 1));
	shadowCol += textureOffset(shadowMap, shadowUv, ivec2(1, 1));

	shadowCol *= (1.0 / 9.0);
	return shadowCol;
}
#endif

//==============================================================================
float calcShadowFactor(in SpotTexLight light, in vec3 fragPosVspace, 
	in mediump sampler2DArrayShadow shadowMapArr, in float layer)
{
	vec4 texCoords4 = light.texProjectionMat * vec4(fragPosVspace, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

#if PCF == 1
	float shadowFactor = pcfLow(shadowMapArr, texCoords3);
#else
	float shadowFactor = texture(shadowMapArr, 
		vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z));
#endif

	return shadowFactor;
}

//==============================================================================
void main()
{
	// Read texture first. Optimize for future out of order HW
	uvec2 msAll = texture(msFai0, vTexCoords).rg;

	// get frag pos in view space
	vec3 fragPosVspace = getFragPosVSpace();

	// Decode MS
	vec3 normal = unpackNormal(unpackHalf2x16(msAll[1]));
	vec4 diffuseAndSpec = unpackUnorm4x8(msAll[0]);
	vec2 specularAll = unpackSpecular(diffuseAndSpec.a);

	// Ambient color
	fColor = diffuseAndSpec.rgb * sceneAmbientColor.rgb;

	// Point lights
	uint pointLightsCount = tiles[vInstanceId].lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
#if __VERSION__ > 430
		uint lightId = tiles[vInstanceId].pointLightIndices[i];
#else
		uint lightId = tiles[vInstanceId].pointLightIndices[i / 4U][i % 4U];
#endif
		PointLight light = pointLights[lightId];

		vec3 ray;
		float att = calcAttenuationFactor(fragPosVspace, light, ray);

		float lambert = calcLambertTerm(normal, ray);

		fColor += calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
			specularAll, normal, light, ray) * (att * lambert);
	}

	// Spot lights
	uint spotLightsCount = tiles[vInstanceId].lightsCount[2];

	for(uint i = 0U; i < spotLightsCount; ++i)
	{
#if __VERSION__ > 430
		uint lightId = tiles[vInstanceId].spotLightIndices[i];
#else
		uint lightId = tiles[vInstanceId].spotLightIndices[i / 4U][i % 4U];
#endif
		SpotLight light = spotLights[lightId];

		vec3 ray;
		float att = calcAttenuationFactor(fragPosVspace, 
			light.lightBase, ray);

		float lambert = calcLambertTerm(normal, ray);

		float spot = calcSpotFactor(light, fragPosVspace);

		vec3 col = calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
			specularAll, normal, light.lightBase, ray);

		fColor += col * (att * lambert * spot);
	}

	// Spot lights with shadow
	uint spotTexLightsCount = tiles[vInstanceId].lightsCount[3];

	for(uint i = 0U; i < spotTexLightsCount; ++i)
	{
#if __VERSION__ > 430
		uint lightId = tiles[vInstanceId].spotTexLightIndices[i];
#else
		uint lightId = tiles[vInstanceId].spotTexLightIndices[i / 4U][i % 4U];
#endif
		SpotTexLight light = spotTexLights[lightId];

		vec3 ray;
		float att = calcAttenuationFactor(fragPosVspace, 
			light.spotLightBase.lightBase, ray);

		float lambert = calcLambertTerm(normal, ray);

		float spot = 
			calcSpotFactor(light.spotLightBase, fragPosVspace);

		float midFactor = att * lambert * spot;

		//if(midFactor > 0.0)
		{
			float shadowmapLayerId = 
				light.spotLightBase.lightBase.diffuseColorShadowmapId.w;

			float shadow = calcShadowFactor(light, 
				fragPosVspace, shadowMapArr, shadowmapLayerId);

			vec3 col = calcPhong(fragPosVspace, diffuseAndSpec.rgb, 
				specularAll, normal, light.spotLightBase.lightBase, ray);

			fColor += col * (midFactor * shadow);
		}
	}

#if GROUND_LIGHT
	fColor += max(dot(normal, groundLightDir.xyz), 0.0) 
		* vec3(0.1, 0.1, 0.0);
#endif

#if 0
	if(tiles[vInstanceId].lightsCount[3] == 666U)
	{
		fColor += vec3(0.5, 0.0, 0.0);
	}
#endif

#if 0
	if(tiles[vInstanceId].lightsCount[2] > 0)
	{
		uint lightId = 0;

		vec3 pureColor = doPhong(fragPosVspace, normal, diffuseAndSpec.rgb, 
			specularAll, slights[lightId].light);

		float spotFactor = calcSpotFactor(slights[lightId], fragPosVspace);

		fColor += pureColor * spotFactor;
	}
#endif

#if 0
	fColor += vec3(slights[0].light.diffuseColorShadowmapId.w,
		slights[1].light.diffuseColorShadowmapId.w, 0.0);
#endif

#if 0
	if(tiles[vInstanceId].lightsCount[0] > 0)
	{
		fColor += vec3(0.0, 0.1, 0.0);
	}
#endif

#if 0
	vec3 tmpc = vec3((vInstanceId % 4) / 3.0, (vInstanceId % 3) / 2.0, 
		(vInstanceId % 2));
	fColor += tmpc / 40.0;
#endif
}
