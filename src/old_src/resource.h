#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "common.h"
#include "engine_class.h"

class texture_t;
class material_t;
class shader_prog_t;
class mesh_data_t;
class model_data_t;
class model_t;


// resource_t
/// resource class
class resource_t: public nc_t
{
	public:
		uint users_num;

		 resource_t(): users_num(0) {}
		~resource_t() {}

		virtual bool Load( const char* ) = 0;
		virtual void Unload() = 0;
};


/// resource namespace
namespace rsrc { // begin namesapce


template< typename type_t > class container_t;


extern container_t<texture_t>     textures;
extern container_t<shader_prog_t> shaders;
extern container_t<material_t>    materials;
extern container_t<mesh_data_t>   mesh_datas;
extern container_t<model_data_t>  model_datas;
extern container_t<model_t>       models;


/// resource container class
template< typename type_t > class container_t: public vec_t<type_t*>
{
	private:
		typedef typename container_t<type_t>::iterator iterator_t; ///< Just to save me time from typing

	public:
		/**
		 * Search in container by pointer
		 * @param x pointer to the object
		 */
		bool Search( type_t* x )
		{
			for( iterator_t it=vec_t<type_t*>::begin(); it<vec_t<type_t*>::end(); it++ )
			{
				if( x == *it ) return true;
			}
			return false;
		}


		/**
		 * Search in container by name
		 * @param name The name of the resource object
		 */
		type_t* Search( const char* name )
		{
			for( iterator_t it=vec_t<type_t*>::begin(); it<vec_t<type_t*>::end(); it++ )
			{
				if( strcmp( name, (*it)->GetName() ) == 0 )
				return *it;
			}
			return NULL;
		}


		/**
		 * Register an object. Throw error if its allready registered
		 * @param x pointer to the object we want to register
		 */
		void Register( type_t* x )
		{
			DEBUG_ERR( Search( x ) ); // the obj must not be allready loaded

			push_back( x );
		}


		/**
		 * UnRegister an object. Try not to call this
		 * @param x pointer to the object we want to unregister
		 */
		void UnRegister( type_t* x )
		{
			uint i;
			for( i=0; i<vec_t<type_t*>::size(); i++ )
			{
				if( (*this)[i] == x )
					break;
			}

			if( i==vec_t<type_t*>::size() )
			{
				ERROR( "Entity is unregistered" );
				return;
			}

			vec_t<type_t*>::erase( vec_t<type_t*>::begin() + i );
		}


		/**
		 * Load an object and register it. If its allready loaded return its pointer
		 * @param fname The filename that initializes the object
		 */
		type_t* Load( const char* fname )
		{
			char* name = CutPath(fname);
			type_t* newt = Search( name );

			// allready loaded
			if( newt != NULL )
			{
				++newt->users_num;
				return newt;
			}

			// not loaded
			newt = new type_t();
			newt->SetName( name );
			if( !newt->Load( fname ) )
			{
				ERROR( "Cannot load \"" << fname << '\"' );
				return NULL;
			}
			newt->users_num = 1;
			Register( newt );
			return newt;
		}


		/**
		 * Unload item. If nobody else uses it then delete it completely
		 * @param x The item we want to unload
		 */
		void Unload( type_t* x )
		{
			DEBUG_ERR( Search( x ) );
			DEBUG_ERR( x->users_num < 1 );

			--x->users_num;

			if( x->users_num == 0 )
			{
				UnRegister( x );
				delete x;
			}
		}
}; // end class container_t


} // end namespace
#endif
