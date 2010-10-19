#ifndef TRF_SCRIPT_CTRL_H
#define TRF_SCRIPT_CTRL_H

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
