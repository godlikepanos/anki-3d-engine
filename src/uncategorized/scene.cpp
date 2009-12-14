#include "scene.h"

namespace scene {

/*
=======================================================================================================================================
DATA                                                                                                                                  =
=======================================================================================================================================
*/
skybox_t skybox;

container_object_t objects;
container_light_t lights;
container_camera_t cameras;
container_mesh_t meshes;
container_smodel_t smodels;


/*
=======================================================================================================================================
UpdateAllWorldStuff                                                                                                                   =
=======================================================================================================================================
*/
void UpdateAllWorldStuff()
{
	DEBUG_ERR( objects.size() > 1024 );
	object_t* queue [1024];
	uint head = 0, tail = 0;
	uint num = 0;


	// put the roots
	for( uint i=0; i<objects.size(); i++ )
		if( objects[i]->parent == NULL )
			queue[tail++] = objects[i]; // queue push

	// loop
	while( head != tail ) // while queue not empty
	{
		object_t* obj = queue[head++]; // queue pop

		obj->UpdateWorldStuff();
		++num;

		for( uint i=0; i<obj->childs.size(); i++ )
			queue[tail++] = obj->childs[i];
	}

	DEBUG_ERR( num != objects.size() );
}


/*
=======================================================================================================================================
RenderAll                                                                                                                             =
=======================================================================================================================================
*/
void RenderAllObjs()
{
	for( uint i=0; i<objects.size(); i++ )
		objects[i]->Render();
}


/*
=======================================================================================================================================
InterpolateAllModels                                                                                                                  =
=======================================================================================================================================
*/
void InterpolateAllModels()
{
	for( uint i=0; i<smodels.size(); i++ )
	{
		smodels[i]->Interpolate();
		smodels[i]->Deform();
	}
}


/*
=======================================================================================================================================
object_t                                                                                                                              =
=======================================================================================================================================
*/
void container_object_t::Register( object_t* x )
{
	RegisterMe( x );
}

void container_object_t::Unregister( object_t* x )
{
	UnregisterMe( x );
}


/*
=======================================================================================================================================
camera_t                                                                                                                              =
=======================================================================================================================================
*/
void container_camera_t::Register( camera_t* x )
{
	RegisterMe( x );
	objects.Register( x );
}

void container_camera_t::Unregister( camera_t* x )
{
	UnregisterMe( x );
	objects.Unregister( x );
}


/*
=======================================================================================================================================
light_t                                                                                                                               =
=======================================================================================================================================
*/
void container_light_t::Register( light_t* x )
{
	RegisterMe( x );
	objects.Register( x );

	if( x->GetType() == light_t::SPOT )
	{
		spot_light_t* projl = static_cast<spot_light_t*>(x);
		cameras.Register( &projl->camera );
	}
}

void container_light_t::Unregister( light_t* x )
{
	UnregisterMe( x );
	objects.Unregister( x );

	if( x->GetType() == light_t::SPOT )
	{
		spot_light_t* projl = static_cast<spot_light_t*>(x);
		cameras.Unregister( &projl->camera );
	}
}


/*
=======================================================================================================================================
smodel_t                                                                                                                               =
=======================================================================================================================================
*/
void container_smodel_t::Register( smodel_t* x )
{
	RegisterMe( x );
	objects.Register( x );
}

void container_smodel_t::Unregister( smodel_t* x )
{
	UnregisterMe( x );
	objects.Unregister( x );
}


/*
=======================================================================================================================================
mesh_t                                                                                                                                =
=======================================================================================================================================
*/
void container_mesh_t::Register( mesh_t* x )
{
	RegisterMe( x );
	objects.Register( x );
}

void container_mesh_t::Unregister( mesh_t* x )
{
	UnregisterMe( x );
	objects.Unregister( x );
}


} // end namespace
