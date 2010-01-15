#ifndef _LIGHT_CONTROLLER_H_
#define _LIGHT_CONTROLLER_H_

#include "common.h"
#include "controller.h"


class light_t;


class light_controller_t: public controller_t
{
	public:
		light_t* light;
		
		light_controller_t( light_t* light_ ): controller(CT_LIGHT), light(light_) {}
		void Update( float ) { /* ToDo */ }
}

#endif
