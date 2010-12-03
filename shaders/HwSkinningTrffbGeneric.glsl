#pragma anki vertShaderBegins

in vec3 position;
in vec3 normal;
in vec4 tangent;
in float vertWeightBonesNum;
in vec4  vertWeightBoneIds;
in vec4  vertWeightWeights;

const int MAX_BONES_PER_MESH = 60;
uniform mat3 skinningRotations[MAX_BONES_PER_MESH];
uniform vec3 skinningTranslations[MAX_BONES_PER_MESH];

#pragma anki transformFeedbackVarying vPosition
out vec3 vPosition;
#pragma anki transformFeedbackVarying vNormal
varying vec3 vNormal;
#pragma anki transformFeedbackVarying vTangent
varying vec4 vTangent;


void main()
{
	mat3 rot = mat3(0.0);
	vec3 tsl = vec3(0.0);

	for(int i=0; i<int(vertWeightBonesNum); i++)
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
{}
