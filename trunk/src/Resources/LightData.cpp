#include <cstring>
#include "LightData.h"
#include "Parser.h"
#include "Texture.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
LightData::LightData():
	Resource(RT_LIGHT_PROPS),
	diffuseCol(0.5),
	specularCol(0.5),
	castsShadow_(false),
	radius(1.0),
	distance(3.0),
	fovX(M::PI/4.0),
	fovY(M::PI/4.0)
{}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void LightData::load(const char* filename)
{
	try
	{
		Scanner scanner(filename);
		const Scanner::Token* token;
		type = LT_NUM;

		while(true)
		{
			token = &scanner.getNextToken();

			// type
			if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "type"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() == Scanner::TC_IDENTIFIER)
				{
					if(!strcmp(token->getValue().getString(), "LT_SPOT"))
					{
						type = LT_SPOT;
					}
					else if(!strcmp(token->getValue().getString(), "LT_POINT"))
					{
						type = LT_POINT;
					}
					else
					{
						PARSER_THROW_EXCEPTION_EXPECTED("LT_SPOT or LT_POINT");
						return false;
					}
				}
			}
			// diffuseCol
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "diffuseCol"))
			{
				Parser::parseMathVector(scanner, diffuseCol);
			}
			// specularCol
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "specularCol"))
			{
				Parser::parseMathVector(scanner, specularCol);
			}
			// radius
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "radius"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
					return false;
				}

				radius = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																															 float(token->getValue().getInt());
			}
			// castsShadow
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "castsShadow"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() == Scanner::TC_IDENTIFIER)
				{
					if(!strcmp(token->getValue().getString(), "true"))
					{
						castsShadow_ = true;
					}
					else if(!strcmp(token->getValue().getString(), "false"))
					{
						castsShadow_ = false;
					}
					else
					{
						PARSER_THROW_EXCEPTION_EXPECTED("true or false");
						return false;
					}
				}
				else
				{
					PARSER_THROW_EXCEPTION_EXPECTED("true or false");
					return false;
				}
			}
			// distance
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "distance"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
					return false;
				}

				distance = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																																 float(token->getValue().getInt());
				type = LT_SPOT;
			}
			// fovX
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "fovX"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
					return false;
				}

				fovX = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																														 float(token->getValue().getInt());
				type = LT_SPOT;
			}
			// fovY
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "fovY"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
					return false;
				}

				fovY = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																														 float(token->getValue().getInt());
				type = LT_SPOT;
			}
			// texture
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "texture"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_STRING)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("string");
					return false;
				}

				texture.loadRsrc(token->getValue().getString());
				texture->setRepeat(false);
				texture->setAnisotropy(0);
				texture->setFiltering(Texture::TFT_LINEAR);
				type = LT_SPOT;
			}
			// end of file
			else if(token->getCode() == Scanner::TC_EOF)
			{
				break;
			}
			// other crap
			else
			{
				PARSER_THROW_EXCEPTION_UNEXPECTED();
				return false;
			}
		} // end while
	}
	catch(std::exception& e)
	{
		THROW_EXCEPTION(e.what());
	}
	
	// sanity checks
	if(type == LT_NUM)
	{
		THROW_EXCEPTION("Forgot to set type");
	}
}
