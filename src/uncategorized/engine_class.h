#ifndef _ENGINE_CLASS_H_
#define _ENGINE_CLASS_H_

#include "common.h"

// nc_t
/// class with name
class nc_t
{
	protected:
		char* name;
		static char unamed[]; ///< The default string that the nc_t::name takes

	public:
		 nc_t();
		~nc_t();

		void SetName( const char* name_ );
		const char* GetName() const { return name; }
};


// data_user_class_t
class data_user_class_t: public nc_t
{};



#endif
