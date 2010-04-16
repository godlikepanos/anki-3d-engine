#ifndef _PARSER_H_
#define _PARSER_H_

#include "Common.h"
#include "Scanner.h"

/**
 * @brief It contains some functions and macros that include code commonly used in parsing
 */
namespace Parser {


//=====================================================================================================================================
// Parser macros                                                                                                                      =
//=====================================================================================================================================
#define PARSE_ERR(x) ERROR( "Parse Error (" << scanner.getScriptName() << ':' << scanner.getLineNmbr() << "): " << x )
#define PARSE_WARN(x) WARNING( "Parse Warning (" << scanner.getScriptName() << ':' << scanner.getLineNmbr() << "): " << x )

// common parser errors
#define PARSE_ERR_EXPECTED(x) PARSE_ERR( "Expected " << x << " and not " << Scanner::getTokenInfo(scanner.getCrntToken()) );
#define PARSE_ERR_UNEXPECTED() PARSE_ERR( "Unexpected token " << Scanner::getTokenInfo(scanner.getCrntToken()) );


//=====================================================================================================================================
// parseArrOfNumbers                                                                                                                  =
//=====================================================================================================================================
/**
 * @brief This template func is used for a common operation of parsing arrays of numbers
 *
 * It parses expressions like this one: { 10 -0.2 123.e-10 -0x0FF } and stores the result in the arr array. The acceptable types
 * (typename Type) are integer or floating point types
 *
 * @param scanner The scanner that we will use
 * @param bracket If true the array starts and ends with brackets eg { 10 0 -1 }
 * @param signs If true the array has numbers that may contain sign
 * @param size The count of numbers of the array wa want to parse
 * @param arr The array that the func returns the numbers
 * @return True if the parsing was successful
*/
template <typename Type> bool parseArrOfNumbers( Scanner& scanner, bool bracket, bool signs, uint size, Type* arr )
{
	const Scanner::Token* token;

	// first of all the left bracket
	if( bracket )
	{
		token = &scanner.getNextToken();
		if( token->getCode() != Scanner::TC_LBRACKET )
		{
			PARSE_ERR_EXPECTED( "{" );
			return false;
		}
	}

	// loop to parse numbers
	for( uint i=0; i<size; i++ )
	{
		token = &scanner.getNextToken();


		// check if there is a sign in front of a number
		bool sign = 0; // sign of the number. 0 for positive and 1 for negative
		if( signs )
		{
			if( token->getCode() == Scanner::TC_MINUS )
			{
				sign = 1;
				token = &scanner.getNextToken();
			}
			else if( token->getCode() == Scanner::TC_PLUS )
			{
				sign = 0;
				token = &scanner.getNextToken();
			}

			// check if not number
			if( token->getCode() != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
		} // end if signs

		// put the number in the arr and do typecasting from int to float
		Type nmbr;

		if( token->getDataType() == Scanner::DT_FLOAT )
			nmbr = static_cast<Type>( token->getValue().getFloat() );
		else
			nmbr = static_cast<Type>( token->getValue().getInt() );

		arr[i] = (sign==0) ? nmbr : -nmbr;
	}

	// the last thing is the right bracket
	if( bracket )
	{
		token = &scanner.getNextToken();
		if( token->getCode() != Scanner::TC_RBRACKET )
		{
			PARSE_ERR_EXPECTED( "}" );
			return false;
		}
	}

	return true;
}


} // end namespace Parser

#endif
