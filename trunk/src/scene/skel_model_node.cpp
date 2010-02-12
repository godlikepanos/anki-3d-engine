#include "skel_model_node.h"
#include "parser.h"
#include "skel_node.h"
#include "mesh_skel_ctrl.h"

/// Create a skel_node and N mesh_nodes that have a mesh_skel_ctrl
void skel_model_node_t::Init( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return;
	const scanner_t::token_t* token;


	//** SKELETON **
	token = &scanner.GetNextToken();
	if( !(token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SKELETON" )) )
	{
		PARSE_ERR_EXPECTED( "identifier SKELETON" );
		return;
	}
	
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_STRING )
	{
		PARSE_ERR_EXPECTED( "string" );
		return;
	}
	
	skel_node = new skel_node_t;
	skel_node->Init( token->value.string );
	AddChild( skel_node );
	
	//** MESHES **
	token = &scanner.GetNextToken();
	if( !(token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "MESHES_NUM" )) )
	{
		PARSE_ERR_EXPECTED( "identifier MESHES_NUM" );
		return;
	}
		
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return;
	}
			
	mesh_nodes.resize( token->value.int_ );
			
	for( uint i=0; i<mesh_nodes.size(); ++i )
	{
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_STRING )
		{
			PARSE_ERR_EXPECTED( "string" );
			return;
		}
				
		mesh_nodes[i] = new mesh_node_t;
		mesh_nodes[i]->Init( token->value.string );
		skel_node->AddChild( mesh_nodes[i] );
		mesh_nodes[i]->mesh_skel_ctrl = new mesh_skel_ctrl_t( skel_node, mesh_nodes[i] );
	}
}
