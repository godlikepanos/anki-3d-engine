#ifndef SKEL_NODE_H
#define SKEL_NODE_H

#include "Common.h"
#include "SceneNode.h"
#include "SkelAnimCtrl.h"
#include "RsrcPtr.h"
#include "Skeleton.h"


/**
 * Scene node that extends the Skeleton resource
 */
class SkelNode: public SceneNode
{
	public:
		RsrcPtr<Skeleton> skeleton; ///< The skeleton resource
		SkelAnimCtrl* skelAnimCtrl; ///< Hold the controller here as well @todo make it reference

		SkelNode();
		void render();
		void init(const char* filename);
};


#endif
