#ifndef _MAP_H_
#define _MAP_H_

#include "common.h"
#include "collision.h"

class mesh_data_t;
class camera_t;


/*
=======================================================================================================================================
octree_t                                                                                                                              =
=======================================================================================================================================
*/
class octree_t
{
	public:
		/// node_t class
		// the class does not contain complicated functions. It mainly holds the data
		class node_t
		{
			public:
				node_t* childs[8];
				aabb_t  bounding_box;

				vec_t<mesh_data_t*>   meshes;
				vec_t< vec_t<uint> > vert_ids;
				vec_t< vec_t<uint> > face_ids;

				node_t() {}
				~node_t() { /*ToDo: when class is finalized add code*/ }

				uint GetMeshesNum() const { return meshes.size(); }
				uint GetFacesNum() const;
		};
		/// end node_t class

	protected:
		// funcs for the tree creation
		bool IsSubdivHeuristicMet( node_t* node ) const;
		void SubdivideNode( node_t* node );
		void CreateRoot( const vec_t<mesh_data_t*>& meshes );

		// frustum funcs
		uint CheckNodeAgainstFrustum( node_t* node, const camera_t& cam ) const;

	public:
		node_t* root;

		void CreateTree( const vec_t<mesh_data_t*>& meshes );
};


/*
=======================================================================================================================================
map_t                                                                                                                                 =
=======================================================================================================================================
*/
class map_t
{
	public:
		vec_t<mesh_data_t*> meshes;
		octree_t             octree;

		bool Load( const char* filename );
		void CreateOctree() { DEBUG_ERR( meshes.size() < 1 ); octree.CreateTree(meshes); };
};


#endif
