#include <stdio.h>
#include <stdarg.h>
#include "object.h"
#include "primitives.h"
#include "renderer.h"
#include "collision.h"


/*
=======================================================================================================================================
Constructor                                                                                                                           =
=======================================================================================================================================
*/
object_t::object_t( type_e type_ )
{
	type = type_;
	parent = NULL;
	translation_lspace = vec3_t( 0.0, 0.0, 0.0 );
	scale_lspace = 1.0;
	rotation_lspace = mat3_t::GetIdentity();
	//mat3_t::ident.Print();
	//rotation_lspace.Print();
	translation_wspace = vec3_t( 0.0, 0.0, 0.0 );
	scale_wspace = 1.0;
	rotation_wspace = mat3_t::GetIdentity();
	bvolume_lspace = NULL;
}


/*
=======================================================================================================================================
GetTypeStr                                                                                                                            =
=======================================================================================================================================
*/
const char* object_t::GetTypeStr() const
{
	switch( type )
	{
		case LIGHT:
			return "LIGHT";
		case CAMERA:
			return "CAMERA";
		case MODEL:
			return "MODEL";
		case MESH:
			return "MESH";
		default:
			return "OTHER";
	}
}


/*
=======================================================================================================================================
RenderAxis                                                                                                                            =
In localspace                                                                                                                         =
=======================================================================================================================================
*/
void object_t::RenderAxis()
{
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_LINES );
		glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
		glVertex3fv( &vec3_t( 0.0, 0.0, 0.0 )[0] );
		glVertex3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );

		glColor3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
		glVertex3fv( &vec3_t( 0.0, 0.0, 0.0 )[0] );
		glVertex3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );

		glColor3fv( &vec3_t( 0.0, 0.0, 1.0 )[0] );
		glVertex3fv( &vec3_t( 0.0, 0.0, 0.0 )[0] );
		glVertex3fv( &vec3_t( 0.0, 0.0, 1.0 )[0] );
	glEnd();
}


/*
=======================================================================================================================================
UpdateWorldTransform                                                                                                                  =
=======================================================================================================================================
*/
void object_t::UpdateWorldTransform()
{
	if( parent )
	{
		/* the original code:
		scale_wspace = parent->scale_wspace * scale_lspace;
		rotation_wspace = parent->rotation_wspace * rotation_lspace;
		translation_wspace = translation_lspace.Transformed( parent->translation_wspace, parent->rotation_wspace, parent->scale_wspace ); */
		CombineTransformations( parent->translation_wspace, parent->rotation_wspace, parent->scale_wspace, translation_lspace, rotation_lspace,
														scale_lspace, translation_wspace, rotation_wspace, scale_wspace );
	}
	else
	{
		scale_wspace = scale_lspace;
		rotation_wspace = rotation_lspace;
		translation_wspace = translation_lspace;
	}

	transformation_wspace = mat4_t( translation_wspace, rotation_wspace, scale_wspace );


	// transform the bvolume
	if( bvolume_lspace != NULL )
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
	}
}


/*
=======================================================================================================================================
Move(s)                                                                                                                               =
Move the object according to it's local axis                                                                                          =
=======================================================================================================================================
*/
void object_t::MoveLocalX( float distance )
{
	vec3_t x_axis = rotation_lspace.GetColumn(0);
	translation_lspace += x_axis * distance;
}

void object_t::MoveLocalY( float distance )
{
	vec3_t y_axis = rotation_lspace.GetColumn(1);
	translation_lspace += y_axis * distance;
}

void object_t::MoveLocalZ( float distance )
{
	vec3_t z_axis = rotation_lspace.GetColumn(2);
	translation_lspace += z_axis * distance;
}


/*
=======================================================================================================================================
AddChilds                                                                                                                             =
=======================================================================================================================================
*/
void object_t::AddChilds( uint amount, ... )
{
	va_list vl;
  va_start(vl, amount);

	for( uint i=0; i<amount; i++ )
	{
		object_t* child = va_arg( vl, object_t* );
		child->parent = this;
		childs.push_back( child );
	}

	va_end(vl);
}


/*
=======================================================================================================================================
MakeParent                                                                                                                            =
make "this" the parent of the obj given as param                                                                                      =
=======================================================================================================================================
*/
void object_t::MakeParent( object_t* obj )
{
	DEBUG_ERR( obj->parent ); // cant have allready a parent
	childs.push_back( obj );
	obj->parent = this;
}
