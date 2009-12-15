
// hardware skinning data
attribute float vert_weight_bones_num;
attribute vec4  vert_weight_bone_ids;
attribute vec4  vert_weight_weights;

const int MAX_BONES_PER_MESH = 60;

uniform mat3 skinning_rotations[ MAX_BONES_PER_MESH ];
uniform vec3 skinning_translations[ MAX_BONES_PER_MESH ];


void HWSkinning( out mat3 _rot, out vec3 _tsl )
{
	_rot = mat3( 0.0 );
	_tsl = vec3( 0.0 );

	for( int i=0; i<int(vert_weight_bones_num); i++ )
	{
		int bone_id = int( vert_weight_bone_ids[i] );
		float weight = vert_weight_weights[i];

		_rot += skinning_rotations[bone_id] * weight;
		_tsl += skinning_translations[bone_id] * weight;
	}
}
