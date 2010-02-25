#ifndef _PARSER_H_
#define _PARSER_H_

#include "Common.h"
#include "Scanner.h"

/*
=======================================================================================================================================
parser macros                                                                                                                         =
=======================================================================================================================================
*/
#define PARSE_ERR(x) ERROR( "PARSE_ERR (" << scanner.getScriptName() << ':' << scanner.getLineNmbr() << "): " << x )
#define PARSE_WARN(x) WARNING( "PARSE_WARN (" << scanner.getScriptName() << ':' << scanner.getLineNmbr() << "): " << x )

// common parser errors
#define PARSE_ERR_EXPECTED(x) PARSE_ERR( "Expected " << x << " and not " << Scanner::getTokenInfo(scanner.getCrntToken()) );
#define PARSE_ERR_UNEXPECTED() PARSE_ERR( "Unexpected token " << Scanner::getTokenInfo(scanner.getCrntToken()) );


/*
=======================================================================================================================================
ParseArrOfNumbers                                                                                                                     =
=======================================================================================================================================
*/
/**
It parses expressions like this one: { 10 -0.2 123.e-10 -0x0FF } and stores the result in the arr array
@param scanner The scanner that we will use
@param size The number of numbers inside the brackets
@param arr The array that the func returns the numbers
@return True if the parsing was success or false otherwise
*/
template <typename Type> bool ParseArrOfNumbers( Scanner& scanner, bool paren, bool signs, uint size, Type* arr )
{
	const Scanner::Token* token;

	// first of all the left bracket
	if( paren )
	{
		token = &scanner.getNextToken();
		if( token->code != Scanner::TC_LBRACKET )
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
			if( token->code == Scanner::TC_MINUS )
			{
				sign = 1;
				token = &scanner.getNextToken();
			}
			else if( token->code == Scanner::TC_PLUS )
			{
				sign = 0;
				token = &scanner.getNextToken();
			}

			// check if not number
			if( token->code != Scanner::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
		} // end if signs

		// put the number in the arr and do typecasting from int to float
		Type nmbr;

		if( token->type == Scanner::DT_FLOAT )
			nmbr = static_cast<Type>( token->value.float_ );
		else
			nmbr = static_cast<Type>( token->value.int_ );

		arr[i] = (sign==0) ? nmbr : -nmbr;
	}

	// the last thing is the right bracket
	if( paren )
	{
		token = &scanner.getNextToken();
		if( token->code != Scanner::TC_RBRACKET )
		{
			PARSE_ERR_EXPECTED( "}" );
			return false;
		}
	}

	return true;
}


#endif
