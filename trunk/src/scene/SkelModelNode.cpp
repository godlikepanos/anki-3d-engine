#include "SkelModelNode.h"
#include "parser.h"
#include "SkelNode.h"
#include "MeshSkelNodeCtrl.h"

/// Create a skelNode and N meshNodes that have a meshSkelCtrl
void SkelModelNode::init( const char* filename )
{
	Scanner scanner;
	if( !scanner.loadFile( filename ) ) return;
	const Scanner::Token* token;


	//** SKELETON **
	token = &scanner.getNextToken();
	if( !(token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "SKELETON" )) )
	{
		PARSE_ERR_EXPECTED( "identifier SKELETON" );
		return;
	}
	
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_STRING )
	{
		PARSE_ERR_EXPECTED( "string" );
		return;
	}
	
	skelNode = new SkelNode;
	skelNode->init( token->value.string );
	addChild( skelNode );
	
	//** MESHES **
	token = &scanner.getNextToken();
	if( !(token->code == Scanner::TC_IDENTIFIER && !strcmp( token->value.string, "MESHES_NUM" )) )
	{
		PARSE_ERR_EXPECTED( "identifier MESHES_NUM" );
		return;
	}
		
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return;
	}
			
	meshNodes.resize( token->value.int_ );
			
	for( uint i=0; i<meshNodes.size(); ++i )
	{
		token = &scanner.getNextToken();
		if( token->code != Scanner::TC_STRING )
		{
			PARSE_ERR_EXPECTED( "string" );
			return;
		}
				
		meshNodes[i] = new MeshNode;
		meshNodes[i]->init( token->value.string );
		skelNode->addChild( meshNodes[i] );
		meshNodes[i]->meshSkelCtrl = new MeshSkelNodeCtrl( skelNode, meshNodes[i] );
	}
}
