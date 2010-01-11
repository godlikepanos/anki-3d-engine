#include "skel_model_node.h"
#include "scanner.h"
#include "parser.h"


void skel_model_node_t::Init( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;

	do
	{
		token = &scanner.GetNextToken();

		//** MESHES **
		if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "MESHES_NUM" ) )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->code != scanner_t::DT_INTEGER )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			
			mesh_nodes.resize( token->value.int_ );
			
			for( uint i=0; i<mesh_nodes.size(); ++i )
			{
				token = &scanner.GetNextToken();
				if( token->code != scanner_t::TC_STRING )
				{
					PARSE_ERR_EXPECTED( "string" );
					return false;
				}
				
				mesh_nodes[i] = new mesh_node_t;
				mesh_nodes[i]->Init( token->value.string );
			}			
		}

		//** SKELETON **
		else if( token->code == scanner_t::TC_IDENTIFIER && !strcmp( token->value.string, "SKELETON" ) )
		{
			if( skel_node )
			{
				PARSE_ERR( "skel_node allready set" );
				return false;
			}

			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_STRING )
			{
				PARSE_ERR_EXPECTED( "string" );
				return false;
			}
			
			skel_node = new skel_node;
			skel_node->Init( token->value.string );
		}

		//** EOF **
		else if( token->code == scanner_t::TC_EOF )
		{
			break;
		}

		//** other crap **
		else
		{
			PARSE_ERR_UNEXPECTED();
			return false;
		}
	}while( true ); // end do


	AddChild( skel_node );
	for( uint i=0; i<mesh_nodes.size(); ++i )
	{
		skel_node->AddChild( mesh_nodes[i] );
		mesh_nodes[i]->skel_controller = new skel_controller( skel_node, mesh_nodes[i] );
	}
	
	return true;
}
