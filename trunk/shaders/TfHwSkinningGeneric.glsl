/// @file
///
/// Switches: NORMAL_ENABLED, TANGENT_ENABLED
#pragma anki vertShaderBegins


//
// Attributes
//
layout(location = 0) in vec3 position;

#if defined(NORMAL_ENABLED)
	layout(location = 1) in vec3 normal;
#endif

#if defined(TANGENT_ENABLED)
	layout(location = 2) in vec4 tangent;
#endif
layout(location = 3) in float vertWeightBonesNum;
layout(location = 4) in vec4 vertWeightBoneIds;
layout(location = 5) in vec4 vertWeightWeights;


//
// Uniforms
// 
const int MAX_BONES_PER_MESH = 60;
uniform mat3 skinningRotations[MAX_BONES_PER_MESH];
uniform vec3 skinningTranslations[MAX_BONES_PER_MESH];


//
// Varyings
//
out vec3 vPosition;

#if defined(NORMAL_ENABLED)
	out vec3 vNormal;
#endif

#if defined(TANGENT_ENABLED)
	out vec4 vTangent;
#endif


void main()
{
	mat3 rot = mat3(0.0);
	vec3 tsl = vec3(0.0);

	for(int i = 0; i < int(vertWeightBonesNum); i++)
	{
		int boneId = int(vertWeightBoneIds[i]);
		float weight = vertWeightWeights[i];

		rot += skinningRotations[boneId] * weight;
		tsl += skinningTranslations[boneId] * weight;
	}
	
	vPosition = (rot * position) + tsl;
	
	#if defined(NORMAL_ENABLED)
		vNormal = rot * normal;
	#endif
	
	#if defined(TANGENT_ENABLED)
		vTangent = vec4(rot * vec3(tangent), tangent.w);
	#endif
}

#pragma anki fragShaderBegins

void main()
{
	// Do nothing
}

