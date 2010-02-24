#ifndef _LIGHT_SCRIPT_CTRL_H_
#define _LIGHT_SCRIPT_CTRL_H_

#include "common.h"
#include "Controller.h"


class Light;


class LightPropsScriptCtrl: public Controller
{
	public:
		Light* light;
		
		LightPropsScriptCtrl( Light* light_ ): controller(CT_LIGHT), light(light_) {}
		void update( float ) { /* ToDo */ }
}

#endif
