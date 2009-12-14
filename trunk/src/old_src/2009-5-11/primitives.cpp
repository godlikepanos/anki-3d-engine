#include "primitives.h"
#include "renderer.h"


/*
=======================================================================================================================================
Constructor                                                                                                                           =
=======================================================================================================================================
*/
object_t::object_t()
{
	parent = NULL;
	local_translation.SetZero();
	local_scale = 1.0f;
	local_rotation.SetIdent();
	world_translation.SetZero();
	world_scale = 1.0f;
	world_rotation.SetIdent();
}


/*
=======================================================================================================================================
RenderAxis                                                                                                                            =
In localspace                                                                                                                         =
=======================================================================================================================================
*/
void object_t::RenderAxis()
{
	r::SetGLState_Wireframe();
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
		world_scale = parent->world_scale * local_scale;
		world_rotation = parent->world_rotation * local_rotation;
		world_translation = local_translation.Transformed( parent->world_translation, parent->world_rotation, parent->world_scale ); */
		CombineTransformations( parent->world_translation, parent->world_rotation, parent->world_scale, local_translation, local_rotation,
														local_scale, world_translation, world_rotation, world_scale );
	}
	else
	{
		world_scale = local_scale;
		world_rotation = local_rotation;
		world_translation = local_translation;
	}

	world_transformation = mat4_t( world_translation, world_rotation, world_scale );
}


/*
=======================================================================================================================================
Move(s)                                                                                                                               =
Move the object according to it's local axis                                                                                          =
=======================================================================================================================================
*/
void object_t::MoveLocalX( float distance )
{
	vec3_t x_axis = local_rotation.GetColumn(0);
	local_translation += x_axis * distance;
}

void object_t::MoveLocalY( float distance )
{
	vec3_t y_axis = local_rotation.GetColumn(1);
	local_translation += y_axis * distance;
}

void object_t::MoveLocalZ( float distance )
{
	vec3_t z_axis = local_rotation.GetColumn(2);
	local_translation += z_axis * distance;
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
		childs.push_back( va_arg( vl, object_t* ) );
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
