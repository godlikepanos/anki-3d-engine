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


// forward decl
namespace rsrc {
template< typename type_t > class container_t;
}


// resource_t
/**
 * Every class that it is considered resource should be derivative of this one. This step is not
 * necessary because of the container_t template but ensures that loading will be made by the
 * resource manager and not the class itself
 */
class resource_t
{
	PROPERTY_R( string, path, GetPath );
	PROPERTY_R( string, name, GetName );
	PROPERTY_R( uint, users_num, GetUsersNum );

	// friends
	friend class rsrc::container_t<texture_t>;
	friend class rsrc::container_t<material_t>;
	friend class rsrc::container_t<shader_prog_t>;
	friend class rsrc::container_t<mesh_data_t>;
	friend class rsrc::container_t<model_data_t>;
	friend class rsrc::container_t<model_t>;

	public:
		virtual bool Load( const char* ) = 0;
		virtual void Unload() = 0;

		resource_t(): users_num(0) {}
		virtual ~resource_t() {};
};


/// resource namespace
namespace rsrc { // begin namesapce


extern container_t<texture_t>     textures;
extern container_t<shader_prog_t> shaders;
extern container_t<material_t>    materials;
extern container_t<mesh_data_t>   mesh_datas;
extern container_t<model_data_t>  model_datas;
extern container_t<model_t>       models;


/// resource container class
template<typename type_t> class container_t: public vec_t<type_t*>
{
	private:
		typedef typename container_t<type_t>::iterator iterator_t; ///< Just to save me time from typing
		typedef vec_t<type_t*> base_class_t;

		/**
		* Search inside the container by name
		* @param name The name of the resource
		* @return The iterator of the content end of vector if not found
		*/
		iterator_t FindByName( const char* name )
		{
			iterator_t it = base_class_t::begin();
			while( it != base_class_t::end() )
			{
				if( (*it).name == name )
					return it;
				++it;
			}

			return it;
		}


		/**
		* Search inside the container by name and path
		* @param name The name of the resource
		* @param path The path of the resource
		* @return The iterator of the content end of vector if not found
		*/
		iterator_t FindByNameAndPath( const char* name, const char* path )
		{
			iterator_t it = base_class_t::begin();
			while( it != base_class_t::end() )
			{
				if( (*it)->name == name && (*it)->path == path )
					return it;
				++it;
			}

			return it;
		}


		/**
		* Search inside the container by pointer
		* @param name The name of the resource object
		* @return The iterator of the content end of vector if not found
		*/
		iterator_t FindByPtr( type_t* ptr )
		{
			iterator_t it = base_class_t::begin();
			while( it != base_class_t::end() )
			{
				if( ptr == (*it) )
					return it;
				++it;
			}

			return it;
		}

		public:
		/**
		* Load an object and register it. If its allready loaded return its pointer
		* @param fname The filename that initializes the object
		*/
		type_t* Load( const char* fname )
		{
			char* name = CutPath( fname );
			string path = GetPath( fname );
			iterator_t it = FindByNameAndPath( name, path.c_str() );

			// if allready loaded then inc the users and return the pointer
			if( it != base_class_t::end() )
			{
				++ (*it)->users_num;
				return (*it);
			}

			// else create new, loaded and update the container
			type_t* new_instance = new type_t();
			new_instance->name = name;
			new_instance->path = path;
			new_instance->users_num = 1;

			if( !new_instance->Load( fname ) )
			{
				ERROR( "Cannot load \"" << fname << '\"' );
				delete new_instance;
				return NULL;
			}
			base_class_t::push_back( new_instance );

			return new_instance;
		}


		/**
		* Unload item. If nobody else uses it then delete it completely
		* @param x Pointer to the instance we want to unload
		*/
		void Unload( type_t* x )
		{
			iterator_t it = FindByPtr( x );
			if( it == base_class_t::end() )
			{
				ERROR( "Cannot find resource with pointer 0x" << x );
				return;
			}

			type_t* del_ = (*it);
			DEBUG_ERR( del_->users_num < 1 ); // WTF?

			--del_->users_num;

			// if no other users then call Unload and update the container
			if( del_->users_num == 0 )
			{
				del_->Unload();
				delete del_;
				base_class_t::erase( it );
			}
		}
}; // end class container_t


} // end namespace
#endif
