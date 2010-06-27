#include <cstring>
#include "SkelModelNode.h"
#include "Parser.h"
#include "SkelNode.h"
#include "MeshSkelNodeCtrl.h"

/// Create a skelNode and N meshNodes that have a meshSkelCtrl
void SkelModelNode::init(const char* filename)
{
	Scanner scanner;
	if(!scanner.loadFile(filename)) return;
	const Scanner::Token* token;


	//** SKELETON **
	token = &scanner.getNextToken();
	if(!(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "SKELETON")))
	{
		PARSE_ERR_EXPECTED("identifier SKELETON");
		return;
	}
	
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_STRING)
	{
		PARSE_ERR_EXPECTED("string");
		return;
	}
	
	skelNode = new SkelNode;
	skelNode->init(token->getValue().getString());
	addChild(skelNode);
	
	//** MESHES **
	token = &scanner.getNextToken();
	if(!(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "MESHES_NUM")))
	{
		PARSE_ERR_EXPECTED("identifier MESHES_NUM");
		return;
	}
		
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return;
	}
			
	meshNodes.resize(token->getValue().getInt());
			
	for(uint i=0; i<meshNodes.size(); ++i)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_STRING)
		{
			PARSE_ERR_EXPECTED("string");
			return;
		}
				
		meshNodes[i] = new MeshNode;
		meshNodes[i]->init(token->getValue().getString());
		skelNode->addChild(meshNodes[i]);
		meshNodes[i]->meshSkelCtrl = new MeshSkelNodeCtrl(skelNode, meshNodes[i]);
	}
}
