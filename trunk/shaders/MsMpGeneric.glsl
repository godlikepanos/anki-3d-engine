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

in vec3 vNormal;
in vec3 vTangent;
in float vTangentW;
in vec2 vTexCoords;
in vec3 vVertPosViewSpace;
// @todo 
in vec3 eye;

layout(location = 0) out vec2 fMsNormalFai;
layout(location = 1) out vec3 fMsDiffuseFai;
layout(location = 2) out vec4 fMsSpecularFai;


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
			vec4 _diffCol4_ = texture2D(diffuseMap, _superTexCoords_);
			if(_diffCol4_.a < alphaTestingTolerance)
			{
				discard;
			}
			_diffColl_ = _diffCol4_.rgb;
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
		vec3 _n_ = normalize(vNormal);
		vec3 _t_ = normalize(vTangent);
		vec3 _b_ = cross(_n_, _t_) * vTangentW;

		mat3 _tbnMat_ = mat3(_t_, _b_, _n_);

		vec3 _nAtTangentspace_ = (texture2D(normalMap, _superTexCoords_).rgb - 0.5) * 2.0;

		vec3 _normal_ = normalize(_tbnMat_ * _nAtTangentspace_);
	#else
		vec3 _normal_ = normalize(vNormal);
	#endif


	//
	// Diffuse Calculations (Part II)
	// If SEM is enabled make some calculations (using the vVertPosViewSpace, environmentMap and the _normal_) and
	/// combine colors of SEM and the _diffColl_
	//
	#if defined(ENVIRONMENT_MAPPING)
		//
		// In case of normal mapping I could play with vertex's normal but this gives better results and its allready
		// computed
		//
		vec3 _u_ = normalize(vVertPosViewSpace);
		vec3 _r_ = reflect(_u_, _normal_);
		_r_.z += 1.0;
		float _m_ = 2.0 * length(_r_);
		vec2 _semTexCoords_ = _r_.xy / _m_ + 0.5;

		vec3 _semCol_ = texture2D(environmentMap, _semTexCoords_).rgb;
		_diffColl_ += _semCol_; // blend existing color with the SEM texture map
	#endif


	//
	// Specular Calculations
	//
	#if defined(SPECULAR_MAPPING)
		vec4 _specularCol_ = vec4(texture2D(specularMap, _superTexCoords_).rgb * specularCol, shininess);
	#else // no specular map
		vec4 _specularCol_ = vec4(specularCol, shininess);
	#endif


	//
	// Final Stage. Write all data
	//
	fMsNormalFai = packNormal(_normal_);
	fMsDiffuseFai = _diffColl_;
	fMsSpecularFai = _specularCol_;

	/*#if defined(HARDWARE_SKINNING)
		gl_FragData[1] = gl_Color;
	#endif*/
}



