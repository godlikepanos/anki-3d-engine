/**
 * @file
 *
 * This a generic shader to fill the deferred shading buffers. You can always build your own if you dont need to write
 * in all the buffers
 */
 
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

#if defined(HARDWARE_SKINNING)
	#pragma anki include "shaders/hw_skinning.glsl"
#endif

// attributes
attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoords;
attribute vec4 tangent;

// uniforms
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelViewMat;
uniform mat3 normalMat;
uniform mat4 modelViewProjectionMat;

// varyings
varying vec3 normal_v2f;
varying vec2 texCoords_v2f;
varying vec3 tangent_v2f;
varying float w_v2f;
varying vec3 vertPosEyeSpace_v2f; ///< For env mapping. AKA view_vector



//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	// calculate the vert pos, normal and tangent

	// if we have hardware skinning then:
	#if defined(HARDWARE_SKINNING)
		mat3 _rot;
		vec3 _tsl;

		HWSkinning(_rot, _tsl);

		normal_v2f = normalMat * (_rot * normal);

		#if NEEDS_TANGENT
			tangent_v2f = normalMat * (_rot * vec3(tangent));
		#endif

		vec3 pos_lspace = (_rot * position) + _tsl;
		gl_Position =  modelViewProjectionMat * vec4(pos_lspace, 1.0);

	// if DONT have hardware skinning
	#else
		normal_v2f = normalMat * normal;

		#if NEEDS_TANGENT
			tangent_v2f = normalMat * vec3(tangent);
		#endif

		gl_Position = modelViewProjectionMat * vec4(position, 1.0);
	#endif


	// calculate the rest

	#if NEEDS_TEX_MAPPING
		texCoords_v2f = texCoords;
	#endif


	#if NEEDS_TANGENT
		w_v2f = tangent.w;
	#endif


	#if defined(ENVIRONMENT_MAPPING) || defined(PARALLAX_MAPPING)
		vertPosEyeSpace_v2f = vec3(modelViewMat * vec4(position, 1.0));
	#endif
}


#pragma anki fragShaderBegins

/**
 * Note: The process of calculating the diffuse color for the diffuse MSFAI is divided into two parts. The first happens
 * before the normal calculation and the other just after it. In the first part we read the texture (or the gl_Color)
 * and we set the _diff_color. In case of grass we discard. In the second part we calculate a SEM color and we combine
 * it with the _diff_color. We cannot put the second part before normal calculation because SEM needs the newNormal.
 * Also we cannot put the first part after normal calculation because in case of grass we will waste calculations for
 * the normal. For that two reasons we split the diffuse calculations in two parts
 */

#pragma anki include "shaders/pack.glsl"


uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D heightMap;
uniform sampler2D environmentMap;
uniform vec3 diffuseCol = vec3(1.0, 1.0, 1.0);
uniform vec3 specularCol = vec3(1.0, 1.0, 1.0);
uniform float shininess = 50.0;

varying vec3 normal_v2f;
varying vec3 tangent_v2f;
varying float w_v2f;
varying vec2 texCoords_v2f;
varying vec3 vertPosEyeSpace_v2f;
// @todo 
varying vec3 eye;



//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	//====================================================================================================================
	// Paralax Mapping Calculations                                                                                      =
	// The code below reads the height map, makes some calculations and returns a new texCoords                          =
	//====================================================================================================================
	#if defined(PARALLAX_MAPPING)
		/*const float _scale = 0.04;
		const float _bias = scale * 0.4;

		vec3 _norm_eye = normalize(eye);

		float _h = texture2D(heightMap, texCoords_v2f).r;
		float _height = _scale * _h - _bias;

		vec2 superTexCoords_v2f = _height * _norm_eye.xy + texCoords_v2f;*/

		vec2 superTexCoords = texCoords_v2f;
		const float maxStepCount = 100.0;
		float nSteps = maxStepCount * length(superTexCoords);

		vec3 dir = vertPosEyeSpace_v2f;
		dir.xy /= 8.0;
		dir /= -nSteps * dir.z;

		float diff0, diff1 = 1.0 - texture2D(heightMap, superTexCoords).a;
		if(diff1 > 0.0)
		{
			do 
			{
				superTexCoords += dir.xy;

				diff0 = diff1;
				diff1 = texture2D(heightMap, superTexCoords).w;
			} while(diff1 > 0.0);

			superTexCoords.xy += (diff1 / (diff0 - diff1)) * dir.xy;
		}
	#else
		#define superTexCoords texCoords_v2f
	#endif


	//====================================================================================================================
	// Diffuse Calculations (Part I)                                                                                     =
	// Get the color from the diffuse map and discard if grass                                                           =
	//====================================================================================================================
	vec3 _diff_color;
	#if defined(DIFFUSE_MAPPING)

		#if defined(ALPHA_TESTING)
			vec4 _diff_color4 = texture2D(diffuseMap, superTexCoords);
			if(_diff_color4.a == 0.0) discard;
			_diff_color = _diff_color4.rgb;
		#else
			_diff_color = texture2D(diffuseMap, superTexCoords).rgb;
		#endif

		_diff_color *= diffuseCol.rgb;
	#else
		_diff_color = diffuseCol.rgb;
	#endif


	//====================================================================================================================
	// Normal Calculations                                                                                               =
	// Either use a normap map and make some calculations or use the vertex normal                                       =
	//====================================================================================================================
	#if defined(NORMAL_MAPPING)
		vec3 _n = normalize(normal_v2f);
		vec3 _t = normalize(tangent_v2f);
		vec3 _b = cross(_n, _t) * w_v2f;

		mat3 tbnMat = mat3(_t,_b,_n);

		vec3 nAtTangentspace = (texture2D(normalMap, superTexCoords).rgb - 0.5) * 2.0;

		vec3 newNormal = normalize(tbnMat * nAtTangentspace);
	#else
		vec3 newNormal = normalize(normal_v2f);
	#endif


	//====================================================================================================================
	// Diffuse Calculations (Part II)                                                                                    =
	// If SEM is enabled make some calculations and combine colors of SEM and the _diff_color                            =
	//====================================================================================================================

	// if SEM enabled make some aditional calculations using the vertPosEyeSpace_v2f, environmentMap and the newNormal
	#if defined(ENVIRONMENT_MAPPING)
		vec3 _u = normalize(vertPosEyeSpace_v2f);
		
		/**
		 * In case of normal mapping I could play with vertex's normal but this gives better results and its allready
		 * computed
		 */
		vec3 _r = reflect(_u, newNormal); 
		
		_r.z += 1.0;
		float _m = 2.0 * length(_r);
		vec2 _sem_texCoords = _r.xy/_m + 0.5;

		vec3 _sem_col = texture2D(environmentMap, _sem_texCoords).rgb;
		_diff_color = _diff_color + _sem_col; // blend existing color with the SEM texture map
	#endif


	//====================================================================================================================
	// Specular Calculations                                                                                             =
	//====================================================================================================================

	// has specular map
	#if defined(SPECULAR_MAPPING)
		vec4 _specular = vec4(texture2D(specularMap, superTexCoords).rgb * specularCol, shininess);
	// no specular map
	#else
		vec4 _specular = vec4(specularCol, shininess);
	#endif


	//====================================================================================================================
	// Final Stage. Write all data                                                                                       =
	//====================================================================================================================
	gl_FragData[0].rg = PackNormal(newNormal);
	gl_FragData[1].rgb = _diff_color;
	gl_FragData[2] = _specular;

	/*#if defined(HARDWARE_SKINNING)
		gl_FragData[1] = gl_Color;
	#endif*/
}



