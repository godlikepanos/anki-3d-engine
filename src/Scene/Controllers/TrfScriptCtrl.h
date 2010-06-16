#ifndef _TRF_SCRIPT_CTRL_H_
#define _TRF_SCRIPT_CTRL_H_

#include "Common.h"
#include "Controller.h"


/// Transformation controlled by a script
class TrfScriptCtrl: public Controller
{
	public:
		SceneNode* node;
	
		TrfScriptCtrl(SceneNode* node_): Controller(CT_TRF), node(node_) {}
		void Update(float) { /* ToDo */ }
};

#endif
