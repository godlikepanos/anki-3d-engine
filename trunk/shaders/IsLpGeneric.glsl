/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"

#define DISCARD 1

/// @name Uniforms
/// @{

/// Watch the placement
layout(std140) uniform uniforms
{
	uniform vec2 planes; ///< for the calculation of frag pos in view space
	/// for the calculation of frag pos in view space
	uniform vec2 limitsOfNearPlane; 
	/// This is an optimization see PpsSsao.glsl and r403 for the clean one
	uniform vec2 limitsOfNearPlane2; 
	uniform float zNear; ///< for the calculation of frag pos in view space
	uniform float lightRadius;
	uniform float shadowMapSize;
	uniform vec3 lightPos; ///< Light pos in eye space
	uniform vec3 lightDiffuseCol;
	uniform vec3 lightSpecularCol;
	uniform mat4 texProjectionMat;
};

uniform usampler2D msFai0;
uniform sampler2D msDepthFai;
uniform sampler2D lightTex;
uniform sampler2DShadow shadowMap;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
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

#if DISCARD
	if(depth == 1.0)
	{
		discard;
	}
#endif

	vec3 fragPosVspace;
	fragPosVspace.z = -planes.y / (planes.x + depth);

	fragPosVspace.xy = (vTexCoords * limitsOfNearPlane2) - limitsOfNearPlane;

	float sc = -fragPosVspace.z / zNear;
	fragPosVspace.xy *= sc;

	return fragPosVspace;
}

//==============================================================================
/// @return The attenuation factor given the distance from the frag to the
/// light source
float getAttenuation(in float fragLightDist)
{
	return clamp(1.0 - sqrt(fragLightDist) / lightRadius, 0.0, 1.0);
	//return 1.0 - fragLightDist * _inv_light_radius;
}

//==============================================================================
/// @return The blurred shadow
float pcfLow(in vec3 shadowUv)
{
	float mapScale = 1.0 / shadowMapSize;
	const int KERNEL_SIZE = 8;
	const vec2 KERNEL[KERNEL_SIZE] = vec2[]
	(
		vec2(1.0, 1.0),
		vec2(1.0, -1.0),
		vec2(-1.0, 1.0),
		vec2(-1.0, -1.0),
		vec2(0.0, 1.0),
		vec2(0.0, -1.0),
		vec2(1.0, 0.0),
		vec2(-1.0, 0.0)
	);
	
	float shadowCol = texture(shadowMap, shadowUv);
	for(int i = 0; i < KERNEL_SIZE; i++)
	{
		vec3 uv = vec3(shadowUv.xy + (KERNEL[i] * mapScale), shadowUv.z);
		shadowCol += texture(shadowMap, uv);
	}
	
	shadowCol *= (1.0 / 9.0);
	return shadowCol;
}

//==============================================================================
/// Performs phong lighting using the MS FAIs and a few other things
/// @param fragPosVspace The fragment position in view space
/// @param fragLightDist Output needed for the attenuation calculation
/// @return The final color
vec3 doPhong(in vec3 fragPosVspace, out float fragLightDist)
{
	// get the vector from the frag to the light
	vec3 frag2LightVec = lightPos - fragPosVspace;

	// Instead of using normalize(frag2LightVec) we brake the operation 
	// because we want fragLightDist for the calc of the attenuation
	fragLightDist = dot(frag2LightVec, frag2LightVec);
	vec3 lightDir = frag2LightVec * inversesqrt(fragLightDist);

	// Read diffuse, normal and the rest from MS
	uvec2 msAll = texture(msFai0, vTexCoords).rg;

	// Read the normal
	vec3 normal = unpackNormal(unpackHalf2x16(msAll[1]));

	// Lambert term
	float lambertTerm = dot(normal, lightDir);

#if DISCARD
	if(lambertTerm < 0.0)
	{
		discard;
	}
#else
	lambertTerm = max(0.0, lambertTerm);
#endif

	// Diffuse
	vec4 diffuseAndSpec = unpackUnorm4x8(msAll[0]);
	diffuseAndSpec.rgb *= lightDiffuseCol;
	vec3 color = diffuseAndSpec.rgb * lambertTerm;

	// Specular
	vec2 specularAll = unpackSpecular(diffuseAndSpec.a);

	vec3 _eyeVec_ = normalize(-fragPosVspace);
	vec3 h = normalize(lightDir + _eyeVec_);
	float specIntensity = pow(max(0.0, dot(normal, h)), specularAll.g);
	color += vec3(specularAll.r) * lightSpecularCol 
		* (specIntensity * lambertTerm);
	
	// end
	return color;
}

//==============================================================================
vec3 doPointLight(in vec3 fragPosVspace)
{
	// The func doPhong calculates the frag to light distance 
	// (fragLightDist) and be cause we need that distance latter for other 
	// calculations we export it
	float fragLightDist;
	vec3 color = doPhong(fragPosVspace, fragLightDist);
	vec3 ret = color * getAttenuation(fragLightDist);
	return ret;
}

//==============================================================================
vec3 doSpotLight(in vec3 fragPosVspace)
{
	vec4 texCoords2 = texProjectionMat * vec4(fragPosVspace, 1.0);
	vec3 texCoords3 = texCoords2.xyz / texCoords2.w;

	if(texCoords2.w > 0.0 &&
	   texCoords3.x > 0.0 &&
	   texCoords3.x < 1.0 &&
	   texCoords3.y > 0.0 &&
	   texCoords3.y < 1.0 &&
	   texCoords2.w < lightRadius)
	{
		// Get shadow
#if SHADOW
#	if PCF
		float shadowCol = pcfLow(texCoords3);
#	else
		float shadowCol = texture(shadowMap, texCoords3);
#	endif

		if(shadowCol == 0.0)
		{
			discard;
		}
#endif

		float fragLightDist;
		vec3 color = doPhong(fragPosVspace, fragLightDist);

		vec3 lightTexCol = textureProj(lightTex, texCoords2.xyz).rgb;
		float att = getAttenuation(fragLightDist);

#if SHADOW
		return lightTexCol * color * (shadowCol * att);
#else
		return lightTexCol * color * att;
#endif
	}
	else
	{
		discard;
	}
}

//==============================================================================
void main()
{
	// get frag pos in view space
	vec3 fragPosVspace = getFragPosVSpace();

#if POINT_LIGHT
	fColor = doPointLight(fragPosVspace);
#elif SPOT_LIGHT
	fColor = doSpotLight(fragPosVspace);
#else
#	error "See file"
#endif

	
	//fColor = vec3(texture(msDepthFai, vTexCoords).r);

	//gl_FragData[0] = gl_FragData[0] - gl_FragData[0] + vec4(1, 0, 1, 1);
	/*#if defined(SPOT_LIGHT)
	fColor = fColor - fColor + vec3(1, 0, 1);
	//gl_FragData[0] = vec4(texture(msDepthFai, vTexCoords).rg), 1.0);
	#endif*/
}
