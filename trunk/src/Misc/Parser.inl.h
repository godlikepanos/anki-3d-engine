#include "Parser.h"
#include "Exception.h"


namespace Parser {


//======================================================================================================================
// parseNumber                                                                                                         =
//======================================================================================================================
template <typename Type>
void parseNumber(Scanner& scanner, bool sign, Type& out)
{
	try
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
				throw PARSER_EXCEPTION_EXPECTED("number or sign");
			}
		}

		// check the number
		if(token->getCode() != Scanner::TC_NUMBER)
		{
			throw PARSER_EXCEPTION_EXPECTED("number");
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
	}
	catch(std::exception& e)
	{
		throw EXCEPTION(e.what());
	}
}


//======================================================================================================================
// parseMathVector                                                                                                     =
//======================================================================================================================
template <typename Type>
void parseMathVector(Scanner& scanner, Type& out)
{
	try
	{
		const Scanner::Token* token = &scanner.getNextToken();
		uint elementsNum = sizeof(Type) / sizeof(float);

		// {
		if(token->getCode() != Scanner::TC_LBRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}

		// numbers
		for(uint i=0; i<elementsNum; i++)
		{
			double d;
			parseNumber(scanner, true, d);
			out[i] = d;
		}

		// }
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_RBRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("}");
		}
	}
	catch(std::exception& e)
	{
		throw EXCEPTION(e.what());
	}
}



} // End namespace Parser
