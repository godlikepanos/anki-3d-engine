#include "spatial.h"



/*
=======================================================================================================================================
UpdateBVolums                                                                                                                         =
=======================================================================================================================================
*/
void spatial_t::UpdateBVolums()
{
	if( local_bvolume == NULL ) return;

	DEBUG_ERR( local_bvolume->type!=bvolume_t::BSPHERE || local_bvolume->type!=bvolume_t::AABB || local_bvolume->type!=bvolume_t::OBB );

	switch( local_bvolume->type )
	{
		case bvolume_t::BSPHERE:
			(*(bsphere_t*)world_bvolume) = (*(bsphere_t*)local_bvolume).Transformed( translation_wspace, rotation_wspace, scale_wspace );
			break;

		case bvolume_t::AABB:
			(*(aabb_t*)world_bvolume) = (*(aabb_t*)local_bvolume).Transformed( translation_wspace, rotation_wspace, scale_wspace );
			break;

		case bvolume_t::OBB:
			(*(obb_t*)world_bvolume) = (*(obb_t*)local_bvolume).Transformed( translation_wspace, rotation_wspace, scale_wspace );
			break;

		default:
			FATAL( "What the fuck" );
	}
}
