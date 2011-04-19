/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki vertShaderBegins

layout(location = 0) in vec2 position; ///< the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}
layout(location = 1) in vec3 viewVector;

out vec2 vTexCoords;
out vec3 vPosition;


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	vPosition = viewVector;
	vTexCoords = position;
	vec2 _vertPosNdc_ = position * 2.0 - 1.0;
	gl_Position = vec4(_vertPosNdc_, 0.0, 1.0);
}


#pragma anki fragShaderBegins

#pragma anki include "shaders/Pack.glsl"

/// @name Uniforms
/// @{
uniform sampler2D msNormalFai, msDiffuseFai, msSpecularFai, msDepthFai;
uniform vec2 planes; ///< for the calculation of frag pos in view space	
uniform vec3 lightPos; ///< Light pos in eye space
uniform float lightRadius;
uniform vec3 lightDiffuseCol;
uniform vec3 lightSpecularCol;
#if defined(SPOT_LIGHT_ENABLED)
	uniform sampler2D lightTex;
	uniform mat4 texProjectionMat;
	#if defined(SHADOW_ENABLED)
		uniform sampler2DShadow shadowMap;
		uniform float shadowMapSize;
	#endif
#endif
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
in vec3 vPosition; ///< for the calculation of frag pos in view space
/// @}

/// @name Output
/// @{
out vec3 fColor;
/// @}


//======================================================================================================================
// getFragPosVSpace                                                                                                    =
//======================================================================================================================
/// @return frag pos in view space
vec3 getFragPosVSpace()
{
	float _depth_ = texture2D(msDepthFai, vTexCoords).r;

	if(_depth_ == 1.0)
	{
		discard;
	}

	vec3 fragPosVspace;
	vec3 vposn = normalize(vPosition);
	fragPosVspace.z = -planes.y / (planes.x + _depth_);
	fragPosVspace.xy = vposn.xy * (fragPosVspace.z / vposn.z);
	return fragPosVspace;
}


//======================================================================================================================
// getAttenuation                                                                                                      =
//======================================================================================================================
/// @return The attenuation factor given the distance from the frag to the light source
float getAttenuation(in float fragLightDist)
{
	return clamp(1.0 - sqrt(fragLightDist) / lightRadius, 0.0, 1.0);
	//return 1.0 - fragLightDist * _inv_light_radius;
}


//======================================================================================================================
// pcfLow                                                                                                              =
//======================================================================================================================
#if defined(SPOT_LIGHT_ENABLED) && defined(SHADOW_ENABLED)

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
	
	float shadowCol = shadow2D(shadowMap, shadowUv).r;
	for(int i = 0; i < KERNEL_SIZE; i++)
	{
		vec3 uv = vec3(shadowUv.xy + (KERNEL[i] * mapScale), shadowUv.z);
		shadowCol += shadow2D(shadowMap, uv).r;
	}
	
	shadowCol *= (1.0 / 9.0);
	return shadowCol;
}

#endif


//======================================================================================================================
// doPhong                                                                                                             =
//======================================================================================================================
/// Performs phong lighting using the MS FAIs and a few other things
/// @param fragPosVspace The fragment position in view space
/// @param fragLightDist Output needed for the attenuation calculation
/// @return The final color
vec3 doPhong(in vec3 fragPosVspace, out float fragLightDist)
{
	// get the vector from the frag to the light
	vec3 frag2LightVec = lightPos - fragPosVspace;

	// Instead of using normalize(frag2LightVec) we brake the operation because we want fragLightDist for the calc of
	// the attenuation
	fragLightDist = dot(frag2LightVec, frag2LightVec);
	vec3 lightDir = frag2LightVec * inversesqrt(fragLightDist);

	// Read the normal
	vec3 normal = unpackNormal(texture2D(msNormalFai, vTexCoords).rg);

	// Lambert term
	float lambertTerm = dot(normal, lightDir);

	if(lambertTerm < 0.0)
	{
		discard;
	}
	//lambertTerm = max(0.0, lambertTerm);

	// Diffuce
	vec3 diffuse = texture2D(msDiffuseFai, vTexCoords).rgb;
	diffuse *= lightDiffuseCol;
	vec3 color = diffuse * lambertTerm;

	// Specular
	vec4 specularMix = texture2D(msSpecularFai, vTexCoords); // the MS specular FAI has the color and the shininess
	vec3 specular = specularMix.xyz;
	float shininess = specularMix.w;

	vec3 _eyeVec_ = normalize(-fragPosVspace);
	vec3 h = normalize(lightDir + _eyeVec_);
	float specIntensity = pow(max(0.0, dot(normal, h)), shininess);
	color += specular * lightSpecularCol * (specIntensity * lambertTerm);
	
	// end
	return color;
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	// get frag pos in view space
	vec3 fragPosVspace = getFragPosVSpace();

	//
	// Point light
	//
	#if defined(POINT_LIGHT_ENABLED)
		// The func doPhong calculates the frag to light distance (fragLightDist) and be cause we need that distance
		// latter for other calculations we export it
		float fragLightDist;
		vec3 color = doPhong(fragPosVspace, fragLightDist);
		fColor = color * getAttenuation(fragLightDist);

	//
	// Spot light
	//
	#elif defined(SPOT_LIGHT_ENABLED)
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
			#if defined(SHADOW_ENABLED)
				#if defined(PCF_ENABLED)
					float shadowCol = pcfLow(texCoords3);
				#else
					float shadowCol = shadow2D(shadowMap, texCoords3).r;
				#endif

				if(shadowCol == 0.0)
				{
					discard;
				}
			#endif

			float fragLightDist;
			vec3 color = doPhong(fragPosVspace, fragLightDist);

			vec3 lightTexCol = texture2DProj(lightTex, texCoords2.xyz).rgb;
			float att = getAttenuation(fragLightDist);

			#if defined(SHADOW_ENABLED)
				fColor = lightTexCol * color * (shadowCol * att);
			#else
				fColor = lightTexCol * color * att;
			#endif
		}
		else
		{
			discard;
		}
	#endif // spot light



	/*#if defined(POINT_LIGHT_ENABLED)
		fColor = fColor - fColor + vec3(1, 0, 1);
	#endif*/
	
	//gl_FragData[0] = gl_FragData[0] - gl_FragData[0] + vec4(1, 0, 1, 1);
	/*#if defined(SPOT_LIGHT_ENABLED)
	fColor = fColor - fColor + vec3(1, 0, 1);
	//gl_FragData[0] = vec4(texture2D(msDepthFai, vTexCoords).rg), 1.0);
	#endif*/
}
