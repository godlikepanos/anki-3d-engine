#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "common.h"
#include "Math.h"
#include "Resource.h"
#include "engine_class.h"


/// Skeleton
class Skeleton: public Resource
{
	public:
		/// Bone
		class Bone: public nc_t
		{
			public:
				static const uint MAX_CHILDS_PER_BONE = 4; ///< Please dont change this
				ushort  id; ///< pos inside the Skeleton::bones vector
				Bone* parent;
				Bone* childs[ MAX_CHILDS_PER_BONE ];
				ushort  childsNum;
				Vec3  head;
				Vec3  tail;

				/* The rotation and translation that transform the bone from bone space to armature space. Meaning that if
				MA = TRS(rotSkelSpace, tslSkelSpace) then head = MA * Vec3(0.0, length, 0.0) and tail = MA * Vec3( 0.0, 0.0, 0.0 )
				We also keep the inverted ones for fast calculations. rotSkelSpaceInv = MA.Inverted().getRotationPart() and NOT
				rotSkelSpaceInv = rotSkelSpace.GetInverted() */
				Mat3 rotSkelSpace;
				Vec3 tslSkelSpace;
				Mat3 rotSkelSpaceInv;
				Vec3 tslSkelSpaceInv;

				 Bone() {}
				~Bone() {}
		};	
	
		Vec<Bone> bones;

		 Skeleton() {}
		~Skeleton() {}
		bool load( const char* filename );
		void unload() { bones.clear(); }
};


#endif
