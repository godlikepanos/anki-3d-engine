#ifndef _NODE_H_
#define _NODE_H_

#include "common.h"

#include "common.h"
#include "gmath.h"


class bvolume_t;
class material_t;


/// Scene Node
class node_t
{
	public:
		enum type_e
		{
			NT_LIGHT,
			NT_CAMERA,
			NT_MESH,
			NT_SKELETON
		};

		vec3_t translation_lspace;
		float  scale_lspace;
		mat3_t rotation_lspace;

		vec3_t translation_wspace;
		float  scale_wspace;
		mat3_t rotation_wspace;

		mat4_t transformation_wspace;

		node_t* parent;
		vec_t<node_t*> childs;

		type_e type;

		bvolume_t* bvolume_lspace;
		bvolume_t* bvolume_wspace;

		bool is_group_node;

		// funcs
		 node_t( type_e type_ );
		~node_t() {};
		virtual void Render() = 0;
		virtual void Init( const char* filename ) = 0;
		virtual void Deinit() = 0;
		virtual void UpdateWorldStuff() { UpdateWorldTransform(); } ///< This update happens only when the object gets moved. Override it if you want more
		void UpdateWorldTransform();
		void RotateLocalX( float ang_degrees ) { rotation_lspace.RotateXAxis( ang_degrees ); }
		void RotateLocalY( float ang_degrees ) { rotation_lspace.RotateYAxis( ang_degrees ); }
		void RotateLocalZ( float ang_degrees ) { rotation_lspace.RotateZAxis( ang_degrees ); }
		void MoveLocalX( float distance );
		void MoveLocalY( float distance );
		void MoveLocalZ( float distance );
		void AddChild( node_t* node );
		void RemoveChild( node_t* node );
};


#endif
