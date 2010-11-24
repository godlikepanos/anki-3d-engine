#include "Model.h"
#include "Parser.h"
#include "Material.h"
#include "Mesh.h"
#include "SkelAnim.h"
#include "MeshData.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


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

				// }
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
		// skeleton
		//
		if(Parser::isIdentifier(token, "skeleton"))
		{
			skeleton.loadRsrc(Parser::parseString(scanner).c_str());
		}
		//
		// skelAnims
		//
		if(Parser::isIdentifier(token, "skelAnims"))
		{
			scanner.getNextToken();

			// {
			if(token->getCode() != Scanner::TC_LBRACKET)
			{
				throw PARSER_EXCEPTION_EXPECTED("{");
			}

			while(true)
			{
				scanner.getNextToken();

				// }
				if(token->getCode() != Scanner::TC_RBRACKET)
				{
					break;
				}
				else if(token->getCode() == Scanner::TC_STRING)
				{
					skelAnims.push_back(RsrcPtr<SkelAnim>());
					skelAnims.back().loadRsrc(token->getValue().getString());
				}
				else
				{
					throw PARSER_EXCEPTION_EXPECTED("} or string");
				}

			}
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

	//
	// Sanity checks
	//
	if(skelAnims.size() > 0 && skeleton.get() == NULL)
	{
		throw EXCEPTION("Model \"" + filename + "\": You have skeleton animations but no skeleton");
	}

	//
	// Create VAOs
	//
	for(Vec<SubModel>::iterator it = subModels.begin(); it != subModels.end(); it++)
	{
		createVao(*it->material, *it->mesh, *it, it->vao);
		createVao(*it->dpMaterial, *it->mesh, *it, it->dpVao);
	}
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
	subModel.material.loadRsrc(material.c_str());
	subModel.dpMaterial.loadRsrc(dpMaterial.c_str());
}


//======================================================================================================================
// createVao                                                                                                           =
//======================================================================================================================
void Model::createVao(const Material& mtl, const Mesh& mesh, SubModel& subModel, Vao*& vao)
{
	vao = new Vao(&subModel);

	if(mtl.getStdAttribVar(Material::SAV_POSITION) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_POSITIONS), *mtl.getStdAttribVar(Material::SAV_POSITION),
		                          3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_NORMAL) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_NORMALS), *mtl.getStdAttribVar(Material::SAV_NORMAL),
		                          3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_TANGENT) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_TANGENTS), *mtl.getStdAttribVar(Material::SAV_TANGENT),
		                          4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_TEX_COORDS) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_TEX_COORDS), *mtl.getStdAttribVar(Material::SAV_TEX_COORDS),
		                          2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONES_NUM) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		                          *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONES_NUM), 1,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(0));
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONE_IDS) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		                          *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONE_IDS), 4,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(4));
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_WEIGHTS) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		                          *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_WEIGHTS), 4,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(20));
	}

	vao->attachElementArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_INDECES));
}
