#ifndef SKEL_NODE_H
#define SKEL_NODE_H

#include <memory>
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
		Vec<Vec3> heads;
		Vec<Vec3> tails;
		Vec<Mat3> boneRotations;
		Vec<Vec3> boneTranslations;

		SkelNode();
		void render();
		void init(const char* filename);

};


#endif
