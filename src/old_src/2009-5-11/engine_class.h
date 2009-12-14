#ifndef _ENGINE_CLASS_H_
#define _ENGINE_CLASS_H_

#include "common.h"


// class with name
class nc_t
{
	protected:
		char* name;

	public:
		 nc_t(): name(NULL) {}
		~nc_t() { if(name) free(name); }

		void SetName( const char* name_ )
		{
			DEBUG_ERR( name != NULL );
			name = (char*)malloc( (strlen(name_) + 1) * sizeof(char) );
			strcpy( name, name_ );

			if( strlen(name) > 20 )
				WARNING( "Big name for: \"" << name << '\"' );
		}

		const char* GetName() const { return name; }
};


// engine class
class engine_class_t: public nc_t
{
	protected:
		uint id;

	public:
		 engine_class_t() {}
		~engine_class_t() {}

		uint GetID() const { return id; }
		void SetID( uint id_ ) { id = id_; }
};


// data_class_t
class data_class_t: public engine_class_t
{
	public:
		uint users_num;

		 data_class_t(): users_num(0) {}
		~data_class_t() {}
};


// runtime_class_t
class runtime_class_t: public engine_class_t
{};



#endif
