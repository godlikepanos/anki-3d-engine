
// hardware skinning data
attribute float vertWeightBonesNum;
attribute vec4  vertWeightBoneIds;
attribute vec4  vertWeightWeights;

const int MAX_BONES_PER_MESH = 60;

uniform mat3 skinningRotations[ MAX_BONES_PER_MESH ];
uniform vec3 skinningTranslations[ MAX_BONES_PER_MESH ];


void HWSkinning( out mat3 _rot, out vec3 _tsl )
{
	_rot = mat3( 0.0 );
	_tsl = vec3( 0.0 );

	for( int i=0; i<int(vertWeightBonesNum); i++ )
	{
		int bone_id = int( vertWeightBoneIds[i] );
		float weight = vertWeightWeights[i];

		_rot += skinningRotations[bone_id] * weight;
		_tsl += skinningTranslations[bone_id] * weight;
	}
}
