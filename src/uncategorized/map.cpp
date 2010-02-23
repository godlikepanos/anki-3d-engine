#include <limits>
#include "map.h"
#include "mesh.h"
#include "Scanner.h"
#include "parser.h"
#include "resource.h"
#include "camera.h"


/*
=======================================================================================================================================
CreateRoot                                                                                                                            =
=======================================================================================================================================
*/
void octree_t::CreateRoot( const vec_t<mesh_t*>& meshes )
{
	DEBUG_ERR( root ); // root should be NULL

	/// get the root's aabb size
	vec3_t min( numeric_limits<float>::max() ), max( numeric_limits<float>::min() );

	for( uint m=0; m<meshes.size(); m++ )
	{
		mesh_t* cmesh = meshes[m];
		for( uint v=0; v<cmesh->vert_coords.size(); v++ )
		{
			const vec3_t& vert_coords = cmesh->vert_coords[v];
			for( int i=0; i<3; i++ )
			{
				if( vert_coords[i] > max[i] )
					max[i] = vert_coords[i];
				else if( vert_coords[i] < min[i] )
					min[i] = vert_coords[i];
			} // end for 3 times
		} // end for all mesh verts
	} // end for all meshes


	/// create a new node
	node_t* node = new node_t;
	node->bounding_box.min = min;
	node->bounding_box.max = max;


	/// create the face and vert ids
	DEBUG_ERR( node->face_ids.size() != 0 || node->vert_ids.size() != 0 || node->meshes.size() != 0 ); // vectors not empty. wrong node init

	node->face_ids.resize( meshes.size() );
	node->vert_ids.resize( meshes.size() );
	node->meshes.resize( meshes.size() );

	for( uint m=0; m<meshes.size(); m++ )
	{
		mesh_t* cmesh = meshes[m];

		// first set the mesh
		node->meshes[m] = cmesh;

		// then set the face_ids
		node->face_ids[m].resize( cmesh->tris.size() );
		for( uint f=0; f<cmesh->tris.size(); f++ )
			node->face_ids[m][f] = f; // simple as taking a shit

		// and last the verts
	}

	/// set root
	root = node;
}



/*
=======================================================================================================================================
GetFacesNum                                                                                                                           =
=======================================================================================================================================
*/
uint octree_t::node_t::GetFacesNum() const
{
	int count = 0;
	for( uint i=0; i<meshes.size(); i++ )
	{
		count += meshes[i]->tris.size();
	}
	return count;
}


/*
=======================================================================================================================================
IsSubdivHeuristicMet                                                                                                                  =
returns true when the used difined heuristic is met that sais that we can subdivide the node. Long story short it returns true when   =
we can subdivide the node further                                                                                                     =
=======================================================================================================================================
*/
bool octree_t::IsSubdivHeuristicMet( node_t* node ) const
{
	if( node->GetFacesNum() < 100 ) return false;

	return true;
}



/*
=======================================================================================================================================
SubdivideNode                                                                                                                         =
subdivides the node and creates max 8 children and then subdivides the children                                                       =
=======================================================================================================================================
*/
void octree_t::SubdivideNode( node_t* node )
{
	if( !IsSubdivHeuristicMet(node) ) return;

	// subdivide the children
	for( int i=0; i<8; i++ )
	{
		if( node->childs[i] == NULL ) continue;

		SubdivideNode( node->childs[i] );
	}
}


/*
=======================================================================================================================================
CreateTree                                                                                                                            =
=======================================================================================================================================
*/
void octree_t::CreateTree( const vec_t<mesh_t*>& meshes )
{
	CreateRoot( meshes );
	SubdivideNode( root );
}


/*
=======================================================================================================================================
CheckNodeAgainstFrustum                                                                                                               =
the func checks the node and returns if its inside the cameras fruntum. It returns 0 if the cube is not inside, 1 if partialy         =
inside and 2 if totaly inside                                                                                                         =
=======================================================================================================================================
*/
uint octree_t::CheckNodeAgainstFrustum( node_t* node, const camera_t& cam ) const
{
	int points_outside_frustum_num = 0;
	const aabb_t& box = node->bounding_box;
	vec3_t box_points[] = { box.max, vec3_t(box.min.x, box.max.y, box.max.z), vec3_t(box.min.x, box.min.y, box.max.z), vec3_t(box.max.x, box.min.y, box.max.z),
	                        box.min, vec3_t(box.min.x, box.max.y, box.min.z), vec3_t(box.min.x, box.min.y, box.min.z), vec3_t(box.max.x, box.min.y, box.min.z), };

	for( int i=0; i<8; i++ )
	{
		for( int j=0; j<6; j++ )
		{
			const plane_t& plane = cam.wspace_frustum_planes[j];

			if( plane.Test( box_points[i] ) < 0.0  )
			{
				++points_outside_frustum_num;
				continue;
			}
		}
	}

	if( points_outside_frustum_num == 8 ) return 0;
	if( points_outside_frustum_num < 8 ) return 1;
	return 2;
}



/**
=======================================================================================================================================
map                                                                                                                                   =
=======================================================================================================================================
*/


/*
=======================================================================================================================================
Load                                                                                                                                  =
=======================================================================================================================================
*/
bool map_t::Load( const char* filename )
{
	DEBUG_ERR( meshes.size() != 0 ); // meshes vector should be empty

	Scanner scanner;
	const Scanner::Token* token;
	if( !scanner.loadFile( filename ) ) return false;

	do
	{
		token = &scanner.getNextToken();

		// strings is what we want in this case... please let it be G-Strings
		if( token->code == Scanner::TC_STRING )
		{
			mesh_t* mesh = rsrc::meshes.Load( token->value.string );
			if( !mesh ) return false;

			meshes.push_back( mesh );
		}
		// end of file
		else if( token->code == Scanner::TC_EOF )
		{
			break;
		}
		// other crap
		else
		{
			PARSE_ERR_UNEXPECTED();
			return false;
		}

	}while( true );

	return true;
}

