#ifndef _NODE_H_
#define _NODE_H_

#include "common.h"

#include "common.h"
#include "gmath.h"


class bvolume_t;
class material_t;
class controller_t;


/// Scene Node
class node_t
{
	// data
	public:
		enum type_e
		{
			NT_LIGHT,
			NT_CAMERA,
			NT_MESH,
			NT_SKELETON,
			NT_SKEL_MODEL
		};

		vec3_t translation_lspace;
		mat3_t rotation_lspace;
		float  scale_lspace;

		vec3_t translation_wspace;
		mat3_t rotation_wspace;
		float  scale_wspace;

		mat4_t transformation_wspace;

		node_t* parent;
		vec_t<node_t*> childs;

		type_e type;

		bvolume_t* bvolume_lspace;
		bvolume_t* bvolume_wspace;

		bool is_group_node;
		
		vec_t<controller_t*> controllers;

	// funcs
	private:
		void CommonConstructorCode(); ///< Cause we cannot call constructor from other constructor
		
	public:
		node_t( type_e type_ ): type(type_) { CommonConstructorCode(); }
		virtual ~node_t() { /* ToDo */ };
		virtual void Render() = 0;
		virtual void Init( const char* ) = 0; ///< Init using a script
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
		void SetLocalTransformation( const vec3_t& t, const mat3_t& r, float s ) { translation_lspace=t; rotation_lspace=r; scale_lspace=s; }
};


#endif
