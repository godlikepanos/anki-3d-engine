// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/*#ifndef ANKI_PHYSICS__RAGDOLL_H_
#define ANKI_PHYSICS__RAGDOLL_H_

#include "anki/physics/PhyCommon.h"


class Ragdoll
{
	public:
		enum
		{
			BP_PELVIS,
			BP_SPINE,
			BP_HEAD,

			BP_LEFrustumType::UPPER_ARM,
			BP_LEFrustumType::LOWER_ARM,
			BP_LEFrustumType::PALM,
			BP_LEFrustumType::UPPER_LEG,
			BP_LEFrustumType::LOWER_LEG,
			BP_LEFrustumType::FOOT,

			BP_RIGHT_UPPER_ARM,
			BP_RIGHT_LOWER_ARM,
			BP_RIGHT_PALM,
			BP_RIGHT_UPPER_LEG,
			BP_RIGHT_LOWER_LEG,
			BP_RIGHT_FOOT,

			BP_NUM
		};

		enum
		{
			JOINT_PELVIS_SPINE,
			JOINT_SPINE_HEAD,

			JOINT_LEFrustumType::SHOULDER,
			JOINT_LEFrustumType::ELBOW,
			JOINT_LEFrustumType::WRIST,
			JOINT_LEFrustumType::HIP,
			JOINT_LEFrustumType::KNEE,
			JOINT_LEFrustumType::ANKLE,

			JOINT_RIGHT_SHOULDER,
			JOINT_RIGHT_ELBOW,
			JOINT_RIGHT_WRIST,
			JOINT_RIGHT_HIP,
			JOINT_RIGHT_KNEE,
			JOINT_RIGHT_ANKLE,

			JOINT_NUM
		};

		btRigidBody*       bodies[BP_NUM];
		btTypedConstraint* joints[JOINT_NUM];
};


#endif */
