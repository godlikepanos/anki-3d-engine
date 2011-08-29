#pragma anki include "shaders/Pack.glsl"


#define MAX_SHININESS 128.0


//==============================================================================
// getNormalFromTexture                                                        =
//==============================================================================
/// @param[in] normal The fragment's normal in view space
/// @param[in] tangent The tangent
/// @param[in] tangent Extra stuff for the tangent
/// @param[in] map The map
/// @param[in] texCoords Texture coordinates
vec3 getNormalFromTexture(in vec3 normal, in vec3 tangent, in float tangentW,
	in sampler2D map, in vec2 texCoords)
{
#if defined(COLOR_PASS)
	vec3 n = normalize(normal);
	vec3 t = normalize(tangent);
	vec3 b = cross(n, t) * tangentW;

	mat3 tbnMat = mat3(t, b, n);

	vec3 nAtTangentspace = (texture(map, texCoords).rgb - 0.5) * 2.0;

	return normalize(tbnMat * nAtTangentspace);
#else
	return vec3(0.0);
#endif
}


//==============================================================================
// getNormalSimple                                                             =
//==============================================================================
/// Just normalize
vec3 getNormalSimple(in vec3 normal)
{
#if defined(COLOR_PASS)
	return normalize(normal);
#else
	return vec3(0.0);
#endif
}


//==============================================================================
// getEnvironmentColor                                                         =
//==============================================================================
/// Environment mapping calculations
/// @param[in] vertPosViewSpace Fragment position in view space
/// @param[in] normal Fragment's normal in view space as well
/// @param[in] map The env map
/// @return The color
vec3 getEnvironmentColor(in vec3 vertPosViewSpace, in vec3 normal,
	in sampler2D map)
{
#if defined(COLOR_PASS)
	// In case of normal mapping I could play with vertex's normal but this 
	// gives better results and its allready computed
	
	vec3 u = normalize(vertPosViewSpace);
	vec3 r = reflect(u, normal);
	r.z += 1.0;
	float m = 2.0 * length(r);
	vec2 semTexCoords = r.xy / m + 0.5;

	vec3 semCol = texture(map, semTexCoords).rgb;
	return semCol;
#else
	return vec3(0.0);
#endif
}


//==============================================================================
// getDiffuseColorAndDoAlphaTesting                                            =
//==============================================================================
/// Using a 4-channel texture and a tolerance discard the fragment if the 
/// texture's alpha is less than the tolerance
/// @param[in] map The diffuse map
/// @param[in] tolerance Tolerance value
/// @param[in] texCoords Texture coordinates
/// @return The RGB channels of the map
vec3 getDiffuseColorAndDoAlphaTesting(
	in sampler2D map,
	in vec2 texCoords,
	in float tolerance)
{
#if defined(COLOR_PASS)
	vec4 col = texture(map, texCoords);
	if(col.a < tolerance)
	{
		discard;
	}
	return col.rgb;
#else
	float a = texture(map, texCoords).a;
	if(a < tolerance)
	{
		discard;
	}
	return vec3(0.0);
#endif
}


//==============================================================================
// readRgbFromTexture                                                          =
//==============================================================================
/// Just read the RGB color from texture
vec3 readRgbFromTexture(in sampler2D tex, in vec2 texCoords)
{
#if defined(COLOR_PASS)
	return texture(tex, texCoords).rgb;
#else
	return vec3(0.0);
#endif
}


//==============================================================================
// add2Vec3                                                                    =
//==============================================================================
vec3 add2Vec3(in vec3 a, in vec3 b)
{
	return a + b;
}


//==============================================================================
// writeFais                                                                   =
//==============================================================================
/// XXX
void writeFais(
	in vec3 diffCol, 
	in vec3 normal, 
	in vec3 specularCol,
	in float shininess, 
	in float blurring)
{
#if defined(COLOR_PASS)
	fMsNormalFai = vec3(packNormal(normal), blurring);
	fMsDiffuseFai = diffCol;
	fMsSpecularFai = vec4(specularCol, shininess / MAX_SHININESS);
#endif
}
