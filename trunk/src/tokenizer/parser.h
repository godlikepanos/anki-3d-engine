#ifndef _PARSER_H_
#define _PARSER_H_

#include "common.h"

/*
=======================================================================================================================================
parser macros                                                                                                                         =
=======================================================================================================================================
*/
#define PARSE_ERR(x) ERROR( "PARSE_ERR (" << scanner.GetScriptName() << ':' << scanner.GetLineNmbr() << "): " << x )
#define PARSE_WARN(x) WARNING( "PARSE_WARN (" << scanner.GetScriptName() << ':' << scanner.GetLineNmbr() << "): " << x )

// common parser errors
#define PARSE_ERR_EXPECTED(x) PARSE_ERR( "Expected " << x << " and not " << scanner_t::GetTokenInfo(scanner.GetCrntToken()) );
#define PARSE_ERR_UNEXPECTED() PARSE_ERR( "Unexpected token " << scanner_t::GetTokenInfo(scanner.GetCrntToken()) );


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
template <typename type_t> bool ParseArrOfNumbers( scanner_t& scanner, bool paren, bool signs, uint size, type_t* arr )
{
	const scanner_t::token_t* token;

	// first of all the left bracket
	if( paren )
	{
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_LBRACKET )
		{
			PARSE_ERR_EXPECTED( "{" );
			return false;
		}
	}

	// loop to parse numbers
	for( uint i=0; i<size; i++ )
	{
		token = &scanner.GetNextToken();


		// check if there is a sign in front of a number
		bool sign = 0; // sign of the number. 0 for positive and 1 for negative
		if( signs )
		{
			if( token->code == scanner_t::TC_MINUS )
			{
				sign = 1;
				token = &scanner.GetNextToken();
			}
			else if( token->code == scanner_t::TC_PLUS )
			{
				sign = 0;
				token = &scanner.GetNextToken();
			}

			// check if not number
			if( token->code != scanner_t::TC_NUMBER )
			{
				PARSE_ERR_EXPECTED( "number" );
				return false;
			}
		} // end if signs

		// put the number in the arr and do typecasting from int to float
		type_t nmbr;

		if( token->type == scanner_t::DT_FLOAT )
			nmbr = static_cast<type_t>( token->value.float_ );
		else
			nmbr = static_cast<type_t>( token->value.int_ );

		arr[i] = (sign==0) ? nmbr : -nmbr;
	}

	// the last thing is the right bracket
	if( paren )
	{
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_RBRACKET )
		{
			PARSE_ERR_EXPECTED( "}" );
			return false;
		}
	}

	return true;
}


#endif
