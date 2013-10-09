#define DEFAULT_FLOAT_PRECISION highp
#pragma anki include "shaders/Common.glsl"

#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"

//==============================================================================
// Variables                                                                   =
//==============================================================================

/// Input
#if TESSELLATION

in highp vec2 teTexCoords;
#if PASS_COLOR
in mediump vec3 teNormal;
in mediump vec4 teTangent;
in mediump vec3 teVertPosViewSpace;
#endif

#else // no TESSELLATION

#if INSTANCING
flat in mediump uint vInstanceId;
#endif

in highp vec2 vTexCoords;
#if PASS_COLOR
in mediump vec3 vNormal;
in mediump vec4 vTangent;
in mediump vec3 vVertPosViewSpace;
#endif

#endif // TESSELLATION

// Output
#if PASS_COLOR
#	if USE_MRT
layout(location = 0) out vec4 fMsFai0;
layout(location = 1) out vec4 fMsFai1;
#	else
layout(location = 0) out uvec2 fMsFai0;
#	endif
#	define fMsFai0_DEFINED
#endif

//==============================================================================
// Functions                                                                   =
//==============================================================================

// Getter
#if PASS_COLOR
#	define getNormal_DEFINED
vec3 getNormal()
{
#if TESSELLATION
	return normalize(teNormal);
#else
	return normalize(vNormal);
#endif
}
#endif

// Getter
#if PASS_COLOR
#	define getTangent_DEFINED
vec4 getTangent()
{
#if TESSELLATION
	return teTangent;
#else
	return vTangent;
#endif
}
#endif

// Getter
#define getTextureCoord_DEFINED
vec2 getTextureCoord()
{
#if TESSELLATION
	return teTexCoords;
#else
	return vTexCoords;
#endif
}

// Getter
#if PASS_COLOR
#	define getPositionViewSpace_DEFINED
vec3 getPositionViewSpace()
{
#if TESSELLATION
	return vec3(0.0);
#else
	return vVertPosViewSpace;
#endif
}
#endif

// Do normal mapping
#if PASS_COLOR
#	define getNormalFromTexture_DEFINED
vec3 getNormalFromTexture(in vec3 normal, in vec4 tangent,
	in sampler2D map, in highp vec2 texCoords)
{
#	if LOD > 0
	return normalize(normal);
#	else
	// First read the texture
	vec3 nAtTangentspace = normalize((texture(map, texCoords).rgb - 0.5) * 2.0);

	vec3 n = normal; // Assume that getNormal() is called
	vec3 t = normalize(tangent.xyz);
	vec3 b = cross(n, t) * tangent.w;

	mat3 tbnMat = mat3(t, b, n);

	return tbnMat * nAtTangentspace;
#	endif
}
#endif

// Do environment mapping
#if PASS_COLOR
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

// Using a 4-channel texture and a tolerance discard the fragment if the 
// texture's alpha is less than the tolerance
#define readTextureRgbAlphaTesting_DEFINED
vec3 readTextureRgbAlphaTesting(
	in sampler2D map,
	in highp vec2 texCoords,
	in float tolerance)
{
#if PASS_COLOR
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

// Just read the RGB color from texture
#if PASS_COLOR
#	define readRgbFromTexture_DEFINED
vec3 readRgbFromTexture(in sampler2D tex, in highp vec2 texCoords)
{
	return vec3(texture(tex, texCoords)).rgb;
}
#endif

// Write the data to FAIs
#if PASS_COLOR
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
