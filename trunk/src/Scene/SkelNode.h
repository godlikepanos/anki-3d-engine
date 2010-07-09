#ifndef _SKEL_NODE_H_
#define _SKEL_NODE_H_

#include "Common.h"
#include "SceneNode.h"
#include "SkelAnimCtrl.h"
#include "RsrcPtr.h"
#include "Skeleton.h"


/**
 * Scene node that extends the @ref Skeleton resource
 */
class SkelNode: public SceneNode
{
	public:
		RsrcPtr<Skeleton> skeleton; ///< The skeleton resource
		SkelAnimCtrl* skelAnimCtrl; ///< Hold the controller here as well @todo make it reference

		SkelNode();
		void render();
		void init(const char* filename);
		void deinit();
};


#endif
