#ifndef _SPATIAL_H_
#define _SPATIAL_H_

#include "common.h"
#include "collision.h"
#include "engine_class.h"
#include "object.h"

class spatial_data_user_class_t: public data_user_class_t {}; // for ambiguity reasons

class spatial_t: public object_t, public spatial_data_user_class_t
{
	public:

		bvolume_t* local_bvolume;
		bvolume_t* world_bvolume;

		spatial_t(): object_t(LIGHT) {}; // ToDo: correct this
		~spatial_t() {};
		void UpdateBVolums();
};


#endif
