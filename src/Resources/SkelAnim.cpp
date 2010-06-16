#include "SkelAnim.h"
#include "Scanner.h"
#include "Parser.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool SkelAnim::load(const char* filename)
{
	Scanner scanner;
	if(!scanner.loadFile(filename)) return false;

	const Scanner::Token* token;


	// keyframes
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return false;
	}
	keyframes.resize(token->getValue().getInt());

	if(!Parser::parseArrOfNumbers(scanner, false, false, keyframes.size(), &keyframes[0])) return false;

	// bones num
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
	{
		PARSE_ERR_EXPECTED("integer");
		return false;
	}
	bones.resize(token->getValue().getInt());

	// poses
	for(uint i=0; i<bones.size(); i++)
	{
		// has anim?
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
		{
			PARSE_ERR_EXPECTED("integer");
			return false;
		}

		// it has
		if(token->getValue().getInt() == 1)
		{
			bones[i].keyframes.resize(keyframes.size());

			for(uint j=0; j<keyframes.size(); ++j)
			{
				// parse the quat
				float tmp[4];
				if(!Parser::parseArrOfNumbers(scanner, false, true, 4, &tmp[0])) return false;
				bones[i].keyframes[j].rotation = Quat(tmp[1], tmp[2], tmp[3], tmp[0]);

				// parse the vec3
				if(!Parser::parseArrOfNumbers(scanner, false, true, 3, &bones[i].keyframes[j].translation[0])) return false;
			}
		}
	} // end for all bones


	framesNum = keyframes[keyframes.size()-1] + 1;

	return true;
}
