#include "Model.h"
#include "Parser.h"
#include "Material.h"
#include "Mesh.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Model::load(const char* filename)
{
	Scanner scanner(filename);
	const Scanner::Token* token = &scanner.getCrntToken();

	while(true)
	{
		scanner.getNextToken();

		//
		// subModels
		//
		if(Parser::isIdentifier(token, "subModels"))
		{
			// {
			scanner.getNextToken();
			if(token->getCode() != Scanner::TC_LBRACKET)
			{
				throw PARSER_EXCEPTION_EXPECTED("{");
			}

			// For all subModels
			while(true)
			{
				scanner.getNextToken();

				if(token->getCode() == Scanner::TC_RBRACKET)
				{
					break;
				}
				else if(Parser::isIdentifier(token, "subModel"))
				{
					parseSubModel(scanner);
				}
				else
				{
					throw PARSER_EXCEPTION_EXPECTED("{ or subModel");
				}
			} // End for all subModels
		}
		//
		// EOF
		//
		else if(token->getCode() == Scanner::TC_EOF)
		{
			break;
		}
		//
		// other crap
		//
		else
		{
			throw PARSER_EXCEPTION_UNEXPECTED();
		}
	} // end while
}


//======================================================================================================================
// parseSubModel                                                                                                       =
//======================================================================================================================
void Model::parseSubModel(Scanner& scanner)
{
	const Scanner::Token* token = &scanner.getCrntToken();

	scanner.getNextToken();

	if(token->getCode() != Scanner::TC_LBRACKET)
	{
		throw PARSER_EXCEPTION_EXPECTED("{");
	}

	Parser::parseIdentifier(scanner, "mesh");
	std::string mesh = Parser::parseString(scanner);

	Parser::parseIdentifier(scanner, "material");
	std::string material = Parser::parseString(scanner);

	Parser::parseIdentifier(scanner, "dpMaterial");
	std::string dpMaterial = Parser::parseString(scanner);

	// Load the stuff
	subModels.push_back(SubModel());
	SubModel& subModel = subModels.back();

	subModel.mesh.loadRsrc(mesh.c_str());
	subModel.mtl.loadRsrc(material.c_str());
	subModel.dpMtl.loadRsrc(dpMaterial.c_str());
}
