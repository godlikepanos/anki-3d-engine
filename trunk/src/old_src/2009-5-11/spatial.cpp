#include "spatial.h"



/*
=======================================================================================================================================
UpdateBVolums                                                                                                                         =
=======================================================================================================================================
*/
void spatial_t::UpdateBVolums()
{
	if( local_bvolume == NULL ) return;

	DEBUG_ERR( local_bvolume->type!=BSPHERE || local_bvolume->type!=AABB || local_bvolume->type!=OBB );

	switch( local_bvolume->type )
	{
		case BSPHERE:
			(*(bsphere_t*)world_bvolume) = (*(bsphere_t*)local_bvolume).Transformed( world_translation, world_rotation, world_scale );
			break;

		case AABB:
			(*(aabb_t*)world_bvolume) = (*(aabb_t*)local_bvolume).Transformed( world_translation, world_rotation, world_scale );
			break;

		case OBB:
			(*(obb_t*)world_bvolume) = (*(obb_t*)local_bvolume).Transformed( world_translation, world_rotation, world_scale );
			break;
	}
}
