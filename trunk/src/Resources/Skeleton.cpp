#include <cstring>
#include "Skeleton.h"
#include "Scanner.h"
#include "Parser.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Skeleton::load(const char* filename)
{
	Scanner scanner(filename);
	const Scanner::Token* token;

	//
	// BONES NUM
	//
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		throw PARSER_EXCEPTION_EXPECTED("integer");
	}
	bones.resize(token->getValue().getInt(), Bone());

	for(uint i=0; i<bones.size(); i++)
	{
		Bone& bone = bones[i];
		bone.id = i;

		///@todo clean
		/*Mat3 m3_(Axisang(-PI/2, Vec3(1,0,0)));
		Mat4 m4_(Vec3(0), m3_, 1.0);*/

		// NAME
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_STRING)
		{
			throw PARSER_EXCEPTION_EXPECTED("string");
		}
		bone.name = token->getValue().getString();

		// head
		Parser::parseMathVector(scanner, bone.head);

		//bone.head = m3_ * bone.head;

		// tail
		Parser::parseMathVector(scanner, bone.tail);

		//bone.tail = m3_ * bone.tail;

		// matrix
		Mat4 m4;
		Parser::parseMathVector(scanner, m4);

		//m4 = m4_ * m4;

		// matrix for real
		bone.rotSkelSpace = m4.getRotationPart();
		bone.tslSkelSpace = m4.getTranslationPart();
		Mat4 MAi(m4.getInverse());
		bone.rotSkelSpaceInv = MAi.getRotationPart();
		bone.tslSkelSpaceInv = MAi.getTranslationPart();

		// parent
		token = &scanner.getNextToken();
		if((token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT) &&
			 (token->getCode() != Scanner::TC_IDENTIFIER || strcmp(token->getValue().getString(), "NULL") != 0))
		{
			throw PARSER_EXCEPTION_EXPECTED("integer or NULL");
		}

		if(token->getCode() == Scanner::TC_IDENTIFIER)
		{
			bone.parent = NULL;
		}
		else
		{
			bone.parent = &bones[token->getValue().getInt()];
		}

		// childs
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
		{
			throw PARSER_EXCEPTION_EXPECTED("integer");
		}

		if(token->getValue().getInt() > Bone::MAX_CHILDS_PER_BONE)
		{
			throw PARSER_EXCEPTION("Childs for bone \"" + bone.getName() + "\" exceed the max");
		}

		bone.childsNum = token->getValue().getInt();
		for(int j=0; j<bone.childsNum; j++)
		{
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
			{
				throw PARSER_EXCEPTION_EXPECTED("integer");
			}
			bone.childs[j] = &bones[token->getValue().getInt()];
		}
	} // for all bones
}
