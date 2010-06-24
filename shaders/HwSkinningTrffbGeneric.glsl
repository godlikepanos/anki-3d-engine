#pragma anki vertShaderBegins

attribute vec3 position;
attribute vec3 normal;
attribute vec4 tangent;
attribute float vertWeightBonesNum;
attribute vec4  vertWeightBoneIds;
attribute vec4  vertWeightWeights;

const int MAX_BONES_PER_MESH = 60;
uniform mat3 skinningRotations[MAX_BONES_PER_MESH];
uniform vec3 skinningTranslations[MAX_BONES_PER_MESH];

#pragma anki transformFeedbackVarying outPosition
varying vec3 outPosition;
#pragma anki transformFeedbackVarying outNormal
varying vec3 outNormal;
#pragma anki transformFeedbackVarying outTangent
varying vec4 outTangent;


void main()
{
	rot = mat3(0.0);
	tsl = vec3(0.0);

	for(int i=0; i<int(vertWeightBonesNum); i++)
	{
		int boneId = int(vertWeightBoneIds[i]);
		float weight = vertWeightWeights[i];

		rot += skinningRotations[boneId] * weight;
		tsl += skinningTranslations[boneId] * weight;
	}
	
	outPosition = (rot * position) + tsl;
	
	#if defined(NORMAL_ENABLED)
		outNormal = rot * normal;
	#endif
	
	#if defined(TANGENT_ENABLED)
		outTangent = vec4(rot * vec3(tangent), tangent.w);
	#endif	
}

#pragma anki fragShaderBegins

void main()
{
}
