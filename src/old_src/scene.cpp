#include "scene.h"

namespace scene {

/*
=======================================================================================================================================
DATA                                                                                                                                  =
=======================================================================================================================================
*/
skybox_t skybox;

vec_t<object_t*> objects;
vec_t<spatial_t*> spatials;
vec_t<light_t*> lights;
vec_t<smodel_t*> smodels;
vec_t<mesh_t*> meshes;
vec_t<camera_t*> cameras;


/*
=======================================================================================================================================
TEMPLATES                                                                                                                             =
=======================================================================================================================================
*/
template <typename type_t> bool Search( vec_t<type_t*>& vec, type_t* x )
{
	typename vec_t<type_t*>::iterator it;
	for( it=vec.begin(); it<vec.end(); it++ )
	{
		if( x == *it )
			return true;
	}
	return false;
}

template <typename type_t, typename base_class_t> void Register( vec_t<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) ); // the obj must not be allready loaded

	x->base_class_t::SetID( vec.size() );
	vec.push_back( x );
}

template <typename type_t, typename base_class_t> void UnRegister( vec_t<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) );

	// decr the ids
	typename vec_t<type_t*>::iterator it;
	for( it=vec.begin()+x->base_class_t::GetID()+1; it<vec.end(); it++ )
	{
		(*it)->base_class_t::SetID( (*it)->base_class_t::GetID()-1 );
	}

	vec.erase( vec.begin() + x->base_class_t::GetID() );
}


/*
=======================================================================================================================================
object_t                                                                                                                              =
=======================================================================================================================================
*/
void Register( object_t* x )
{
	Register< object_t, data_user_class_t >( objects, x );
}

void UnRegister( object_t* x )
{
	UnRegister< object_t, data_user_class_t >( objects, x );
}


/*
=======================================================================================================================================
mesh_t                                                                                                                                =
=======================================================================================================================================
*/
void Register( mesh_t* x )
{
	Register< mesh_t, mesh_data_user_class_t >( meshes, x );
	Register( dynamic_cast<object_t*>(x) );
}

void UnRegister( mesh_t* x )
{
	UnRegister< mesh_t, mesh_data_user_class_t >( meshes, x );
	UnRegister( dynamic_cast<object_t*>(x) );
}


/*
=======================================================================================================================================
model_t                                                                                                                               =
=======================================================================================================================================
*/
void Register( smodel_t* x )
{
	Register<smodel_t, model_data_user_class_t>( smodels, x );
	Register( (object_t*)x );
}

void UnRegister( smodel_t* x )
{
	UnRegister<smodel_t, model_data_user_class_t>( smodels, x );
	UnRegister( (object_t*)x );
}


/*
=======================================================================================================================================
light_t                                                                                                                               =
=======================================================================================================================================
*/
void Register( light_t* x )
{
	Register<light_t, light_data_user_class_t>( lights, x );
	Register( (object_t*)x );

	if( x->GetType() == light_t::SPOT )
	{
		spot_light_t* projl = static_cast<spot_light_t*>(x);
		Register( (camera_t*)&projl->camera );
	}
}

void UnRegister( light_t* x )
{
	UnRegister<light_t, light_data_user_class_t>( lights, x );
	UnRegister( (object_t*)x );

	if( x->GetType() == light_t::SPOT )
	{
		spot_light_t* projl = static_cast<spot_light_t*>(x);
		UnRegister( (camera_t*)&projl->camera );
	}
}


/*
=======================================================================================================================================
camera_t                                                                                                                              =
=======================================================================================================================================
*/
void Register( camera_t* x )
{
	Register<camera_t, camera_data_user_class_t>( cameras, x );
	Register( (object_t*)x );
}

void UnRegister( camera_t* x )
{
	UnRegister<camera_t, camera_data_user_class_t>( cameras, x );
	UnRegister( (object_t*)x );
}


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


} // end namespace
