#include <algorithm>
#include "node.h"
#include "renderer.h"
#include "collision.h"


//=====================================================================================================================================
// Constructor                                                                                                                        =
//=====================================================================================================================================
node_t::node_t( type_e type_ )
{
	type = type_;
	parent = NULL;
	translation_lspace = vec3_t( 0.0, 0.0, 0.0 );
	scale_lspace = 1.0;
	rotation_lspace = mat3_t::GetIdentity();
	translation_wspace = vec3_t( 0.0, 0.0, 0.0 );
	scale_wspace = 1.0;
	rotation_wspace = mat3_t::GetIdentity();
	bvolume_lspace = NULL;
}


//=====================================================================================================================================
// UpdateWorldTransform                                                                                                               =
//=====================================================================================================================================
void node_t::UpdateWorldTransform()
{
	if( parent )
	{
		/* the original code:
		scale_wspace = parent->scale_wspace * scale_lspace;
		rotation_wspace = parent->rotation_wspace * rotation_lspace;
		translation_wspace = translation_lspace.Transformed( parent->translation_wspace, parent->rotation_wspace, parent->scale_wspace ); */
		CombineTransformations( parent->translation_wspace, parent->rotation_wspace, parent->scale_wspace,
		                        translation_lspace, rotation_lspace, scale_lspace,
		                        translation_wspace, rotation_wspace, scale_wspace );
	}
	else // else copy
	{
		scale_wspace = scale_lspace;
		rotation_wspace = rotation_lspace;
		translation_wspace = translation_lspace;
	}

	transformation_wspace = mat4_t( translation_wspace, rotation_wspace, scale_wspace );


	// transform the bvolume
	/*if( bvolume_lspace != NULL )
	{
		DEBUG_ERR( bvolume_lspace->type!=bvolume_t::BSPHERE && bvolume_lspace->type!=bvolume_t::AABB && bvolume_lspace->type!=bvolume_t::OBB );

		switch( bvolume_lspace->type )
		{
			case bvolume_t::BSPHERE:
			{
				bsphere_t sphere = static_cast<bsphere_t*>(bvolume_lspace)->Transformed( translation_wspace, rotation_wspace, scale_wspace );
				*static_cast<bsphere_t*>(bvolume_wspace) = sphere;
				break;
			}

			case bvolume_t::AABB:
			{
				aabb_t aabb = static_cast<aabb_t*>(bvolume_lspace)->Transformed( translation_wspace, rotation_wspace, scale_wspace );
				*static_cast<aabb_t*>(bvolume_wspace) = aabb;
				break;
			}

			case bvolume_t::OBB:
			{
				obb_t obb = static_cast<obb_t*>(bvolume_lspace)->Transformed( translation_wspace, rotation_wspace, scale_wspace );
				*static_cast<obb_t*>(bvolume_wspace) = obb;
				break;
			}

			default:
				FATAL( "What the fuck" );
		}
	}*/
}


//=====================================================================================================================================
// Move(s)                                                                                                                            =
// Move the object according to it's local axis                                                                                       =
//=====================================================================================================================================
void node_t::MoveLocalX( float distance )
{
	vec3_t x_axis = rotation_lspace.GetColumn(0);
	translation_lspace += x_axis * distance;
}

void node_t::MoveLocalY( float distance )
{
	vec3_t y_axis = rotation_lspace.GetColumn(1);
	translation_lspace += y_axis * distance;
}

void node_t::MoveLocalZ( float distance )
{
	vec3_t z_axis = rotation_lspace.GetColumn(2);
	translation_lspace += z_axis * distance;
}


//=====================================================================================================================================
// AddChild                                                                                                                           =
//=====================================================================================================================================
void node_t::AddChild( node_t* node )
{
	if( node->parent != NULL )
	{
		ERROR( "Node allready have parent" );
		return;
	}

	node->parent = this;
	childs.push_back( node );
}


//=====================================================================================================================================
// RemoveChild                                                                                                                        =
//=====================================================================================================================================
void node_t::RemoveChild( node_t* node )
{
	vec_t<node_t*>::iterator it = find( childs.begin(), childs.end(), node );
	if( it == childs.end() )
	{
		ERROR( "Child not found" );
		return;
	}

	node->parent = NULL;
	childs.erase( it );
}
