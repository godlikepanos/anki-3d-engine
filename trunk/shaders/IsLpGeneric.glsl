/**
 * @file
 *
 * Illumination stage lighting pass general shader program
 */

#pragma anki vertShaderBegins

#pragma anki attribute viewVector 1
attribute vec3 viewVector;
#pragma anki attribute position 0
attribute vec2 position; ///< the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}

varying vec2 texCoords;
varying vec3 vpos;


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	vpos = viewVector;
	texCoords = position;
	vec2 _vertPosNdc_ = position * 2.0 - 1.0;
	gl_Position = vec4(_vertPosNdc_, 0.0, 1.0);
}


#pragma anki fragShaderBegins

#pragma anki include "shaders/Pack.glsl"

/*
 * Uniforms
 */
uniform sampler2D msNormalFai, msDiffuseFai, msSpecularFai, msDepthFai;
uniform vec2 planes; ///< for the calculation of frag pos in view space	
uniform vec3 lightPos; ///< Light pos in eye space
uniform float lightInvRadius; ///< An opt
uniform vec3 lightDiffuseCol;
uniform vec3 lightSpecularCol;
#if defined(SPOT_LIGHT_ENABLED)
	uniform sampler2D lightTex;
	uniform mat4 texProjectionMat;
	#if defined(SHADOW_ENABLED)
		uniform sampler2DShadow shadowMap;
	#endif
#endif

/*
 * Varyings
 */
varying vec2 texCoords;
varying vec3 vpos; ///< for the calculation of frag pos in view space


//======================================================================================================================
// getFragPosVSpace                                                                                                    =
//======================================================================================================================
/**
 * @return frag pos in view space
 */
vec3 getFragPosVSpace()
{
	float _depth_ = texture2D(msDepthFai, texCoords).r;

	if(_depth_ == 1.0)
		discard;

	vec3 _fragPosVspace_;
	vec3 _vposn_ = normalize(vpos);
	_fragPosVspace_.z = -planes.y / (planes.x + _depth_);
	_fragPosVspace_.xy = _vposn_.xy * (_fragPosVspace_.z / _vposn_.z);
	return _fragPosVspace_;
}


//======================================================================================================================
// getAttenuation                                                                                                      =
//======================================================================================================================
/**
 * @return The attenuation factor fiven the distance from the frag to the light
 * source
 */
float getAttenuation(in float _fragLightDist_)
{
	return clamp(1.0 - lightInvRadius * sqrt(_fragLightDist_), 0.0, 1.0);
	//return 1.0 - _fragLightDist_ * _inv_light_radius;
}


#if defined(SPOT_LIGHT_ENABLED) && defined(SHADOW_ENABLED)


//======================================================================================================================
// pcfLow                                                                                                              =
//======================================================================================================================
/**
 * @return The blurred shadow
 */
float pcfLow(in vec3 _shadowUv_)
{
	const float _mapScale_ = 1.0 / SHADOWMAP_SIZE;
	const int _kernelSize_ = 8;
	const vec2 _kernel_[_kernelSize_] = vec2[]
	(
		vec2(_mapScale_, _mapScale_),
		vec2(_mapScale_, -_mapScale_),
		vec2(-_mapScale_, _mapScale_),
		vec2(-_mapScale_, -_mapScale_),
		vec2(0.0, _mapScale_),
		vec2(0.0, -_mapScale_),
		vec2(_mapScale_, 0.0),
		vec2(-_mapScale_, 0.0)
	);
	
	float _shadowCol_ = shadow2D(shadowMap, _shadowUv_).r;
	for(int i=0; i<_kernelSize_; i++)
	{
		vec3 _uvCoord_ = vec3(_shadowUv_.xy + _kernel_[i], _shadowUv_.z);
		_shadowCol_ += shadow2D(shadowMap, _uvCoord_).r;
	}
	
	_shadowCol_ *= 1.0/9.0;
	return _shadowCol_;
}

#endif


//======================================================================================================================
// doPhong                                                                                                             =
//======================================================================================================================
/**
 * Performs phong lighting using the MS FAIs and a few other things
 * @param _fragPosVspace_ The fragment position in view space
 * @param _fragLightDist_ Output needed for the attenuation calculation
 * @return The final color
 */
vec3 doPhong(in vec3 _fragPosVspace_, out float _fragLightDist_)
{
	// get the vector from the frag to the light
	vec3 _frag2LightVec_ = lightPos - _fragPosVspace_;

	/*
	 * Instead of using normalize(_frag2LightVec_) we brake the operation because we want _fragLightDist_ for the calc of
	 * the attenuation
	 */
	_fragLightDist_ = dot(_frag2LightVec_, _frag2LightVec_);
	vec3 _lightDir_ = _frag2LightVec_ * inversesqrt(_fragLightDist_);

	/*
	 * Read the normal
	 */
	vec3 _normal_ = unpackNormal(texture2D(msNormalFai, texCoords).rg);

	/*
	 * Lambert term
	 */
	float _lambertTerm_ = dot(_normal_, _lightDir_);

	if(_lambertTerm_ < 0.0)
		discard;
	//_lambertTerm_ = max(0.0, _lambertTerm_);

	/*
	 * Diffuce
	 */
	vec3 _diffuse_ = texture2D(msDiffuseFai, texCoords).rgb;
	_diffuse_ = (_diffuse * lightDiffuseCol);
	vec3 _color_ = _diffuse_ * _lambertTerm_;

	/*
	 * Specular
	 */
	vec4 _specularMix_ = texture2D(msSpecularFai, texCoords); // the MS specular FAI has the color and the shininess
	vec3 _specular_ = _specularMix_.xyz;
	float _shininess_ = _specularMix_.w;

	vec3 _eyeVec_ = normalize(-_fragPosVspace_);
	vec3 _h_ = normalize(_lightDir_ + _eyeVec_);
	float _specIntensity_ = pow(max(0.0, dot(_normal_, _h_)), _shininess_);
	_color_ += _specular_ * lightSpecularCol * (_specIntensity_ * _lambertTerm_);
	
	/*
	 * end
	 */
	return _color_;
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main()
{
	// get frag pos in view space
	vec3 fragPosVspace = getFragPosVSpace();

	/*
	 * Point light
	 */
	#if defined(POINT_LIGHT_ENABLED)
		/*
		 * The func phong calculates the frag to light distance (_fragLightDist_) and be cause we need that distance
		 * latter for other calculations we export it
		 */
		float _fragLightDist_;
		vec3 _color_ = doPhong(fragPosVspace, _fragLightDist_);
		gl_FragData[0] = vec4(_color_ * getAttenuation(_fragLightDist_), 1.0);

	/*
	 * Spot light
	 */
	#elif defined(SPOT_LIGHT_ENABLED)
		vec4 _texCoords2_ = texProjectionMat * vec4(fragPosVspace, 1.0);
		vec3 _texCoords3_ = _texCoords2_.xyz / _texCoords2_.w;

		if(_texCoords2_.w > 0.0 &&
		   _texCoords3_.x > 0.0 &&
		   _texCoords3_.x < 1.0 &&
		   _texCoords3_.y > 0.0 &&
		   _texCoords3_.y < 1.0 &&
		   _texCoords2_.w < 1.0/lightInvRadius)
		{
			/*
			 * Get shadow
			 */
			#if defined(SHADOW_ENABLED)
				#if defined(PCF_ENABLED)
					float _shadowCol_ =  shadow2D(shadowMap, _texCoords3_).r;
				#else
					float _shadowCol_ = pcfOff(_texCoords3_);
				#endif

				if(_shadowCol_ == 0.0)
					discard;
			#endif

			float _fragLightDist_;
			vec3 _color_ = doPhong(fragPosVspace, _fragLightDist_);

			vec3 _lightTexCol_ = texture2DProj(lightTex, _texCoords2_.xyz).rgb;
			float _att_ = getAttenuation(_fragLightDist_);

			#if defined(SHADOW_ENABLED)
				gl_FragData[0] = vec4(_lightTexCol_ * _color_ * (_shadowCol_ * _att_), 1.0);
			#else
				gl_FragData[0] = vec4(_color_ * _texel_ * _att_, 1.0);
			#endif
		}
		else
		{
			discard;
		}
	#endif // spot light



	/*#if defined(POINT_LIGHT_ENABLED)
		gl_FragData[0] = gl_FragData[0] - gl_FragData[0] + vec4(1, 0, 1, 1);
	#endif*/
	
	//gl_FragData[0] = gl_FragData[0] - gl_FragData[0] + vec4(1, 0, 1, 1);
	/*#if defined(SPOT_LIGHT_ENABLED)
	gl_FragData[0] = gl_FragData[0] - gl_FragData[0] + vec4(texture2D(msDepthFai, texCoords).r);
	//gl_FragData[0] = vec4(texture2D(msDepthFai, texCoords).rg), 1.0);
	#endif*/
}
