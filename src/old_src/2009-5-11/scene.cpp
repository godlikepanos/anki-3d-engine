#include "scene.h"

namespace scene {

/*
=======================================================================================================================================
DATA                                                                                                                                  =
=======================================================================================================================================
*/
skybox_t skybox;

vector<object_t*> objects;
vector<spatial_t*> spatials;
vector<light_t*> lights;
vector<model_t*> models;
vector<mesh_t*> meshes;
vector<camera_t*> cameras;


/*
=======================================================================================================================================
TEMPLATES                                                                                                                             =
=======================================================================================================================================
*/
template <typename type_t> bool Search( vector<type_t*>& vec, type_t* x )
{
	typename vector<type_t*>::iterator it;
	for( it=vec.begin(); it<vec.end(); it++ )
	{
		if( x == *it )
			return true;
	}
	return false;
}

template <typename type_t, typename base_class_t> void Register( vector<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) ); // the obj must not be allready loaded

	x->base_class_t::SetID( vec.size() );
	vec.push_back( x );
}

template <typename type_t, typename base_class_t> void UnRegister( vector<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) );

	// decr the ids
	typename vector<type_t*>::iterator it;
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
	Register< object_t, runtime_class_t >( objects, x );
}

void UnRegister( object_t* x )
{
	Register< object_t, runtime_class_t >( objects, x );
}


/*
=======================================================================================================================================
model_t                                                                                                                               =
=======================================================================================================================================
*/
void Register( model_t* x )
{
	Register<model_t, model_runtime_class_t>( models, x );
	Register( (object_t*)x );
}

void UnRegister( model_t* x )
{
	UnRegister<model_t, model_runtime_class_t>( models, x );
	UnRegister( (object_t*)x );
}


/*
=======================================================================================================================================
light_t                                                                                                                               =
=======================================================================================================================================
*/
void Register( light_t* x )
{
	Register<light_t, light_runtime_class_t>( lights, x );
	Register( (object_t*)x );
}

void UnRegister( light_t* x )
{
	UnRegister<light_t, light_runtime_class_t>( lights, x );
	UnRegister( (object_t*)x );
}


/*
=======================================================================================================================================
camera_t                                                                                                                              =
=======================================================================================================================================
*/
void Register( camera_t* x )
{
	Register<camera_t, camera_runtime_class_t>( cameras, x );
	Register( (object_t*)x );
}

void UnRegister( camera_t* x )
{
	UnRegister<camera_t, camera_runtime_class_t>( cameras, x );
	UnRegister( (object_t*)x );
}


/*
=======================================================================================================================================
UpdateAllWorldTrfs                                                                                                                    =
=======================================================================================================================================
*/
void UpdateAllWorldTrfs()
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

		obj->UpdateWorldTransform();
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
	for( uint i=0; i<models.size(); i++ )
		models[i]->Interpolate();
}


/*
=======================================================================================================================================
UpdateAllLights                                                                                                                       =
=======================================================================================================================================
*/
void UpdateAllLights()
{
	for( uint i=0; i<lights.size(); i++ )
		lights[i]->Update();
}


/*
=======================================================================================================================================
UpdateAllCameras                                                                                                                      =
=======================================================================================================================================
*/
void UpdateAllCameras()
{
	vector<camera_t*>::iterator it;
	for( it=cameras.begin(); it<cameras.end(); it++ )
	{
		(*it)->Update();
	}
}

} // end namespace
