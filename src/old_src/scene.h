#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "primitives.h"
#include "spatial.h"
#include "lights.h"
#include "mesh.h"
#include "smodel.h"
#include "skybox.h"

namespace scene {

// data
extern vec_t<object_t*> objects;
extern vec_t<spatial_t*> spatials;
extern vec_t<light_t*> lights;
extern vec_t<smodel_t*> smodels;
extern vec_t<mesh_t*> meshes;
extern vec_t<camera_t*> cameras;

extern skybox_t skybox;
inline vec3_t GetAmbientColor() { return vec3_t( 0.1, 0.05, 0.05 )*1; }


// object_t
extern void Register( object_t* obj );
extern void UnRegister( object_t* obj );

// spatial_t
extern void Register( spatial_t* spa );
extern void UnRegister( spatial_t* spa );

// light_t
extern void Register( light_t* light );
extern void UnRegister( light_t* light );

// mesh_t
extern void Register( mesh_t* mesh );
extern void UnRegister( mesh_t* mesh );

// model_t
extern void Register( smodel_t* mdl );
extern void UnRegister( smodel_t* mdl );

// camera_t
extern void Register( camera_t* mdl );
extern void UnRegister( camera_t* mdl );

// MISC funcs
extern void UpdateAllWorldStuff();
extern void RenderAllObjs();
extern void InterpolateAllModels();


/// entities container class
template< typename type_t > class container_t
{
	private:
		typedef typename vector<type_t*>::iterator iterator_t; ///< Just to save me time from typing

		vec_t<type_t*> container; ///< The vector that holds pointers to the all the resources of type type_t


		/**
		Register only x. Throw error if its allready registered
		@param x pointer to the object we want to register
		*/
		void RegisterMe( type_t* x )
		{
			DEBUG_ERR( Search( x ) ); // the obj must not be allready loaded

			x->SetID( container.size() );
			container.push_back( x );
		}


		/**
		Unregister an object. Try not to call this
		@param x pointer to the object we want to unregister
		*/
		void UnregisterMe( type_t* x )
		{
			DEBUG_ERR( Search( x ) );

			// decr the ids
			for( iterator_t it=container.begin()+x->GetID()+1; it<container.end(); it++ )
			{
				(*it)->SetID( (*it)->GetID()-1 );
			}

			container.erase( container.begin() + x->GetID() );
		}

	public:
		/**
		Search in container by pointer
		@param x pointer to the object
		*/
		bool Search( type_t* x )
		{
			for( iterator_t it=container.begin(); it<container.end(); it++ )
			{
				if( x == *it ) return true;
			}
			return false;
		}

		virtual void Resister( type_t* x ) = 0;

		virtual void Unregister( type_t* x ) = 0;

}; // end class container_t

} // end namespace
#endif
