#ifndef _LIGHT_SCRIPT_CTRL_H_
#define _LIGHT_SCRIPT_CTRL_H_

#include "common.h"
#include "controller.h"


class light_t;


class light_script_ctrl_t: public controller_t
{
	public:
		light_t* light;
		
		light_controller_t( light_t* light_ ): controller(CT_LIGHT), light(light_) {}
		void Update( float ) { /* ToDo */ }
}

#endif
