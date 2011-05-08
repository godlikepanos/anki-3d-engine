/// @file
/// 
/// This a generic shader to fill the deferred shading buffers. You can always build your own if you dont need to write
/// in all the buffers
/// 
/// Control defines:
/// DIFFUSE_MAPPING, NORMAL_MAPPING, SPECULAR_MAPPING, PARALLAX_MAPPING, ENVIRONMENT_MAPPING, ALPHA_TESTING
 
#if defined(ALPHA_TESTING) && !defined(DIFFUSE_MAPPING)
	#error "Cannot have ALPHA_TESTING without DIFFUSE_MAPPING"
#endif
 
#if defined(DIFFUSE_MAPPING) || defined(NORMAL_MAPPING) || defined(SPECULAR_MAPPING)
	#define NEEDS_TEX_MAPPING 1
#else
	#define NEEDS_TEX_MAPPING 0
#endif


#if defined(NORMAL_MAPPING) || defined(PARALLAX_MAPPING)
	#define NEEDS_TANGENT 1
#else
	#define NEEDS_TANGENT 0
#endif


#pragma anki vertShaderBegins

/// @name Attributes
/// @{
in vec3 position;
in vec3 normal;
#if NEEDS_TEX_MAPPING
	in vec2 texCoords;
#endif
#if NEEDS_TANGENT
	in vec4 tangent;
#endif
/// @}

/// @name Uniforms
/// @{
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelViewMat;
uniform mat3 normalMat;
uniform mat4 modelViewProjectionMat;
/// @}

/// @name Varyings
/// @{
out vec3 vNormal;
out vec2 vTexCoords;
out vec3 vTangent;
out float vTangentW;
out vec3 vVertPosViewSpace; ///< For env mapping. AKA view vector
/// @}



//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	// calculate the vert pos, normal and tangent
	vNormal = normalMat * normal;

	#if NEEDS_TANGENT
		vTangent = normalMat * vec3(tangent);
	#endif

	gl_Position = modelViewProjectionMat * vec4(position, 1.0);

	// calculate the rest

	#if NEEDS_TEX_MAPPING
		vTexCoords = texCoords;
	#endif


	#if NEEDS_TANGENT
		vTangentW = tangent.w;
	#endif


	#if defined(ENVIRONMENT_MAPPING) || defined(PARALLAX_MAPPING)
		vVertPosViewSpace = vec3(modelViewMat * vec4(position, 1.0));
	#endif
}


#pragma anki fragShaderBegins

/// @note The process of calculating the diffuse color for the diffuse MSFAI is divided into two parts. The first
/// happens before the normal calculation and the other just after it. In the first part we read the texture (or the
/// gl_Color) and we set the _diffColl_. In case of grass we discard. In the second part we calculate a SEM color and
/// we combine it with the _diffColl_. We cannot put the second part before normal calculation because SEM needs
/// the _normal_. Also we cannot put the first part after normal calculation because in case of grass we will waste
/// calculations for the normal. For that two reasons we split the diffuse calculations in two parts

#pragma anki include "shaders/Pack.glsl"


#if defined(DIFFUSE_MAPPING)
	uniform sampler2D diffuseMap;
#endif
#if defined(NORMAL_MAPPING)
	uniform sampler2D normalMap;
#endif
#if defined(SPECULAR_MAPPING)
	uniform sampler2D specularMap;
#endif
#if defined(PARALLAX_MAPPING)
	uniform sampler2D heightMap;
#endif
#if defined(ENVIRONMENT_MAPPING)
	uniform sampler2D environmentMap;
#endif
uniform float shininess = 50.0;
uniform vec3 diffuseCol = vec3(1.0, 0.0, 1.0);
uniform vec3 specularCol = vec3(1.0, 0.0, 1.0);
#if defined(ALPHA_TESTING)
	uniform float alphaTestingTolerance = 0.5; ///< Below this value the pixels are getting discarded 
#endif
uniform float blurring = 0.0;

in vec3 vNormal;
in vec3 vTangent;
in float vTangentW;
in vec2 vTexCoords;
in vec3 vVertPosViewSpace;
// @todo 
in vec3 eye;

layout(location = 0) out vec3 fMsNormalFai;
layout(location = 1) out vec3 fMsDiffuseFai;
layout(location = 2) out vec4 fMsSpecularFai;

const float MAX_SHININESS = 128.0;


//======================================================================================================================
// Normal funcs                                                                                                        =
//======================================================================================================================
/// @param[in] normal The fragment's normal in view space
/// @param[in] tangent The tangent
/// @param[in] tangent Extra stuff for the tangent
/// @param[in] map The map
/// @param[in] texCoords Texture coordinates
vec3 getNormalUsingMap(in vec3 normal, in vec3 tangent, in float tangentW, in sampler2D map, in vec2 texCoords)
{
		vec3 n = normalize(normal);
		vec3 t = normalize(tangent);
		vec3 b = cross(n, t) * tangentW;

		mat3 tbnMat = mat3(t, b, n);

		vec3 nAtTangentspace = (texture2D(map, texCoords).rgb - 0.5) * 2.0;

		return normalize(tbnMat * nAtTangentspace);
}

/// Just normalize
vec3 getNormalSimple(in vec3 normal)
{
	return normalize(normal);
}


//======================================================================================================================
// doEnvMapping                                                                                                        =
//======================================================================================================================
/// Environment mapping calculations
/// @param[in] vertPosViewSpace Fragment position in view space
/// @param[in] normal Fragment's normal in view space as well
/// @param[in] map The env map
/// @return The color
vec3 doEnvMapping(in vec3 vertPosViewSpace, in vec3 normal, in sampler2D map)
{
	// In case of normal mapping I could play with vertex's normal but this gives better results and its allready
	// computed
	
	vec3 u = normalize(vertPosViewSpace);
	vec3 r = reflect(u, normal);
	r.z += 1.0;
	float m = 2.0 * length(r);
	vec2 semTexCoords = r.xy / m + 0.5;

	vec3 semCol = texture2D(map, semTexCoords).rgb;
	return semCol;
}


//======================================================================================================================
// doAlpha                                                                                                             =
//======================================================================================================================
/// Using a 4-channel texture and a tolerance discard the fragment if the texture's alpha is less than the tolerance
/// @param[in] map The diffuse map
/// @param[in] tolerance Tolerance value
/// @param[in] texCoords Texture coordinates
/// @return The RGB channels of the map
vec3 doAlpha(in sampler2D map, in float tolerance, in vec2 texCoords)
{
	vec4 col = texture2D(map, texCoords);
	if(col.a < tolerance)
	{
		discard;
	}
	return col.rgb;
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	//
	// Paralax Mapping Calculations
	// The code below reads the height map, makes some calculations and returns a new texCoords
	//
	#if defined(PARALLAX_MAPPING)
		/*const float _scale = 0.04;
		const float _bias = scale * 0.4;

		vec3 _norm_eye = normalize(eye);

		float _h = texture2D(heightMap, vTexCoords).r;
		float _height = _scale * _h - _bias;

		vec2 _superTexCoords__v2f = _height * _norm_eye.xy + vTexCoords;*/

		vec2 _superTexCoords_ = vTexCoords;
		const float maxStepCount = 100.0;
		float nSteps = maxStepCount * length(_superTexCoords_);

		vec3 dir = vVertPosViewSpace;
		dir.xy /= 8.0;
		dir /= -nSteps * dir.z;

		float diff0, diff1 = 1.0 - texture2D(heightMap, _superTexCoords_).a;
		if(diff1 > 0.0)
		{
			do 
			{
				_superTexCoords_ += dir.xy;

				diff0 = diff1;
				diff1 = texture2D(heightMap, _superTexCoords_).w;
			} while(diff1 > 0.0);

			_superTexCoords_.xy += (diff1 / (diff0 - diff1)) * dir.xy;
		}
	#else
		#define _superTexCoords_ vTexCoords
	#endif


	//
	// Diffuse Calculations (Part I)
	// Get the color from the diffuse map and discard if alpha testing is on and alpha is zero
	//
	vec3 _diffColl_;
	#if defined(DIFFUSE_MAPPING)

		#if defined(ALPHA_TESTING)
			_diffColl_ = doAlpha(diffuseMap, alphaTestingTolerance, _superTexCoords_);
		#else // no alpha
			_diffColl_ = texture2D(diffuseMap, _superTexCoords_).rgb;
		#endif

		_diffColl_ *= diffuseCol.rgb;
	#else // no diff mapping
		_diffColl_ = diffuseCol.rgb;
	#endif


	//
	// Normal Calculations
	// Either use a normap map and make some calculations or use the vertex normal
	//
	#if defined(NORMAL_MAPPING)
		vec3 _normal_ = getNormalUsingMap(vNormal, vTangent, vTangentW, normalMap, _superTexCoords_);
	#else
		vec3 _normal_ = getNormalSimple(vNormal);
	#endif


	//
	// Diffuse Calculations (Part II)
	// If SEM is enabled make some calculations (using the vVertPosViewSpace, environmentMap and the _normal_) and
	// combine colors of SEM and the _diffColl_
	//
	#if defined(ENVIRONMENT_MAPPING)
		// blend existing color with the SEM texture map
		_diffColl_ += doEnvMapping(vVertPosViewSpace, _normal_, environmentMap); 
	#endif


	//
	// Specular Calculations
	//
	#if defined(SPECULAR_MAPPING)
		vec4 _specularCol_ = vec4(texture2D(specularMap, _superTexCoords_).rgb * specularCol,
		                                    shininess / MAX_SHININESS);
	#else // no specular map
		vec4 _specularCol_ = vec4(specularCol, shininess / MAX_SHININESS);
	#endif


	//
	// Final Stage. Write all data
	//
	fMsNormalFai = vec3(packNormal(_normal_), blurring);
	fMsDiffuseFai = _diffColl_;
	fMsSpecularFai = _specularCol_;
}

