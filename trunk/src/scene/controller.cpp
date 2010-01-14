#include "controller.h"
#include "scene.h"

controller_t::controller_t( type_e type_ ): 
	type(type_) 
{
	scene::RegisterController( this );
}

controller_t::~controller_t()
{
	scene::UnregisterController( this );
}
