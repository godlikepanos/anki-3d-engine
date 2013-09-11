#define DEFAULT_FLOAT_PRECISION highp
#pragma anki include "shaders/CommonFrag.glsl"

#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"

//==============================================================================
// Variables                                                                   =
//==============================================================================

/// @name Varyings
/// @{
#define vTexCoords_DEFINED
in highp vec2 vTexCoords;

#if defined(PASS_COLOR)
in vec3 vNormal;
#	define vNormal_DEFINED
in vec3 vTangent;
#	define vTangent_DEFINED
in float vTangentW;
#	define vTangentW_DEFINED
in vec3 vVertPosViewSpace;
#	define vVertPosViewSpace_DEFINED
#endif
/// @}

/// @name Fragment out
/// @{
#if defined(PASS_COLOR)
#	if USE_MRT
layout(location = 0) out vec4 fMsFai0;
layout(location = 1) out vec4 fMsFai1;
#	else
layout(location = 0) out uvec2 fMsFai0;
#	endif
#	define fMsFai0_DEFINED
#endif
/// @}

//==============================================================================
// Functions                                                                   =
//==============================================================================

/// @param[in] normal The fragment's normal in view space
/// @param[in] tangent The tangent
/// @param[in] tangent Extra stuff for the tangent
/// @param[in] map The map
/// @param[in] texCoords Texture coordinates
#if defined(PASS_COLOR)
#	define getNormalFromTexture_DEFINED
vec3 getNormalFromTexture(in vec3 normal, in vec3 tangent, in float tangentW,
	in sampler2D map, in highp vec2 texCoords)
{
#	if LOD > 0
	return normalize(normal);
#	else
	// First read the texture
	vec3 nAtTangentspace = (texture(map, texCoords).rgb - 0.5) * 2.0;

	vec3 n = normalize(normal);
	vec3 t = normalize(tangent);
	vec3 b = cross(n, t) * tangentW;

	mat3 tbnMat = mat3(t, b, n);

	return normalize(tbnMat * nAtTangentspace);
#	endif
}
#endif

/// Just normalize
#if defined(PASS_COLOR)
#	define getNormalSimple_DEFINED
vec3 getNormalSimple(in vec3 normal)
{
	return normalize(normal);
}
#endif

/// Environment mapping calculations
/// @param[in] vertPosViewSpace Fragment position in view space
/// @param[in] normal Fragment's normal in view space as well
/// @param[in] map The env map
/// @return The color
#if defined(PASS_COLOR)
#	define getEnvironmentColor_DEFINED
vec3 getEnvironmentColor(in vec3 vertPosViewSpace, in vec3 normal,
	in sampler2D map)
{
	// In case of normal mapping I could play with vertex's normal but this 
	// gives better results and its allready computed
	
	vec3 u = normalize(vertPosViewSpace);
	vec3 r = reflect(u, normal);
	r.z += 1.0;
	float m = 2.0 * length(r);
	vec2 semTexCoords = r.xy / m + 0.5;

	vec3 semCol = texture(map, semTexCoords).rgb;
	return semCol;
}
#endif

/// Using a 4-channel texture and a tolerance discard the fragment if the 
/// texture's alpha is less than the tolerance
/// @param[in] map The diffuse map
/// @param[in] tolerance Tolerance value
/// @param[in] texCoords Texture coordinates
/// @return The RGB channels of the map
#define readTextureRgbAlphaTesting_DEFINED
vec3 readTextureRgbAlphaTesting(
	in sampler2D map,
	in highp vec2 texCoords,
	in float tolerance)
{
#if defined(PASS_COLOR)
	vec4 col = vec4(texture(map, texCoords));
	if(col.a < tolerance)
	{
		discard;
	}
	return col.rgb;
#else // Depth
#	if LOD > 0
	return vec3(0.0);
#	else
	float a = float(texture(map, texCoords).a);
	if(a < tolerance)
	{
		discard;
	}
	return vec3(0.0);
#	endif
#endif
}

/// Just read the RGB color from texture
#if defined(PASS_COLOR)
#	define readRgbFromTexture_DEFINED
vec3 readRgbFromTexture(in sampler2D tex, in highp vec2 texCoords)
{
	return vec3(texture(tex, texCoords)).rgb;
}
#endif

/// Write the data to FAIs
#if defined(PASS_COLOR)
#	define writeFais_DEFINED
void writeFais(
	in vec3 diffColor, // from 0 to 1
	in vec3 normal, 
	in vec2 specularComponent, // Streangth and shininess
	in float blurring)
{
	writeGBuffer(
		diffColor, normal, specularComponent.x, specularComponent.y,
		fMsFai0
#if USE_MRT
		, fMsFai1
#endif
		);
}
#endif
