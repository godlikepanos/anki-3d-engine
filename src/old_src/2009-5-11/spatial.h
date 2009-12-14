#ifndef _SPATIAL_H_
#define _SPATIAL_H_

#include "common.h"
#include "primitives.h"
#include "collision.h"
#include "engine_class.h"

class spatial_runtime_class_t: public runtime_class_t {}; // for ambiguity reasons

class spatial_t: public object_t, public spatial_runtime_class_t
{
	public:

		bvolume_t* local_bvolume;
		bvolume_t* world_bvolume;

		spatial_t() {};
		~spatial_t() {};
		void UpdateBVolums();
};


#endif
