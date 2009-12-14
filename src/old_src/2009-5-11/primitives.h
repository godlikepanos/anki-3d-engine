#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

#include <math.h>
#include "common.h"
#include "math.h"
#include "engine_class.h"

// vertex_t
class vertex_t
{
	public:
		vec3_t coords;
		vec3_t normal;
};

// triangle_t
class triangle_t
{
	public:
		uint   vert_ids[3]; // an array with the vertex indexes in the mesh class
		vec3_t normal;
};



/*
=======================================================================================================================================
object_t                                                                                                                              =
=======================================================================================================================================
*/
class object_t: public runtime_class_t
{
	public:
		vec3_t local_translation;
		float  local_scale;
		mat3_t local_rotation;

		vec3_t world_translation;
		float  world_scale;
		mat3_t world_rotation;

		mat4_t world_transformation;

		object_t* parent;
		vector<object_t*> childs;

		// funcs
		 object_t();
		~object_t() {};
		virtual void Render() = 0;
		void RenderAxis();
		void UpdateWorldTransform();
		void RotateLocalX( float ang_degrees ) { local_rotation.RotateXAxis( ang_degrees ); }
		void RotateLocalY( float ang_degrees ) { local_rotation.RotateYAxis( ang_degrees ); }
		void RotateLocalZ( float ang_degrees ) { local_rotation.RotateZAxis( ang_degrees ); }
		void MoveLocalX( float distance );
		void MoveLocalY( float distance );
		void MoveLocalZ( float distance );
		void AddChilds( uint amount, ... );
		void MakeParent( object_t* obj );
};

























#endif
