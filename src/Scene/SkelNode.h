#ifndef _SKEL_NODE_H_
#define _SKEL_NODE_H_

#include "Common.h"
#include "Node.h"
#include "Controller.h"
#include "Math.h"


/**
 * @brief Scene node that extends the @ref Skeleton resource
 */
class SkelNode: public Node
{
	public:
		class Skeleton* skeleton; ///< The skeleton resource
		class SkelAnimCtrl* skelAnimCtrl; ///< Hold the controller here as well

		SkelNode();
		void render();
		void init( const char* filename );
		void deinit();
};


#endif
