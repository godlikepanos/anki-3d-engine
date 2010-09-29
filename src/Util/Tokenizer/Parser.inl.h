#include "Parser.h"

namespace Parser {


//======================================================================================================================
// parseNumber                                                                                                         =
//======================================================================================================================
template <typename Type>
bool parseNumber(Scanner& scanner, bool sign, Type& out)
{
	const Scanner::Token* token = &scanner.getNextToken();
	bool negative = false;

	// check the sign if any
	if(sign)
	{
		if(token->getCode() == Scanner::TC_PLUS)
		{
			negative = false;
			token = &scanner.getNextToken();
		}
		else if(token->getCode() == Scanner::TC_MINUS)
		{
			negative = true;
			token = &scanner.getNextToken();
		}
		else if(token->getCode() != Scanner::TC_NUMBER)
		{
			PARSE_ERR_EXPECTED("number or sign");
			return false;
		}
	}

	// check the number
	if(token->getCode() != Scanner::TC_NUMBER)
	{
		PARSE_ERR_EXPECTED("number");
		return false;
	}

	if(token->getDataType() == Scanner::DT_FLOAT)
	{
		double d = negative ? -token->getValue().getFloat() : token->getValue().getFloat();
		out = static_cast<Type>(d);
	}
	else
	{
		ulong l = negative ? -token->getValue().getInt() : token->getValue().getInt();
		out = static_cast<Type>(l);
	}

	return true;
}


//======================================================================================================================
// parseMathVector                                                                                                     =
//======================================================================================================================
template <typename Type>
bool parseMathVector(Scanner& scanner, Type& out)
{
	const Scanner::Token* token = &scanner.getNextToken();
	uint elementsNum = sizeof(Type) / sizeof(float);

	// {
	if(token->getCode() != Scanner::TC_LBRACKET)
	{
		PARSE_ERR_EXPECTED("{");
		return false;
	}

	// numbers
	for(uint i=0; i<elementsNum; i++)
	{
		double d;
		if(!parseNumber(scanner, true, d))
		{
			return false;
		}
		out[i] = d;
	}


	// }
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_RBRACKET)
	{
		PARSE_ERR_EXPECTED("}");
		return false;
	}

	return true;
}



} // End namespace Parser
