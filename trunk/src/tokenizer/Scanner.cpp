#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include <iomanip>
#include "Scanner.h"


#define SERROR(x) ERROR( "SCAN_ERR (" << scriptName << ':' << lineNmbr << "): " << x )


//=====================================================================================================================================
// Token                                                                                                                            =
//=====================================================================================================================================
Scanner::Token::Token( const Token& b ): code(b.code), type(b.type)
{
	switch( b.type )
	{
		case Scanner::DT_FLOAT:
			value.float_ = b.value.float_;
			break;
		case Scanner::DT_INT:
			value.int_ = b.value.int_;
			break;
		case Scanner::DT_CHAR:
			value.char_ = b.value.char_;
			break;
		case Scanner::DT_STR:
			value.string = b.value.string;
			break;	
	}
	memcpy( asString, b.asString, Scanner::MAX_SCRIPT_LINE_LEN*sizeof(char) );
}


//=====================================================================================================================================
// data vars                                                                                                                          =
//=====================================================================================================================================
char Scanner::eofChar = 0x7F;


//=====================================================================================================================================
// reserved words grouped by length                                                                                                   =
//=====================================================================================================================================
Scanner::ResWord Scanner::rw2 [] =
{
	{"ke", TC_KE}, {NULL, TC_ERROR}
};

Scanner::ResWord Scanner::rw3 [] =
{
	{"key", TC_KEY}, {NULL, TC_ERROR}
};

Scanner::ResWord Scanner::rw4 [] =
{
	{"keyw", TC_KEYW}, {NULL, TC_ERROR}
};

Scanner::ResWord Scanner::rw5 [] =
{
	{"keywo", TC_KEYWO}, {NULL, TC_ERROR}
};

Scanner::ResWord Scanner::rw6 [] =
{
	{"keywor", TC_KEYWOR}, {NULL, TC_ERROR}
};

Scanner::ResWord Scanner::rw7 [] =
{
	{"keyword", TC_KEYWORD}, {NULL, TC_ERROR}
};

Scanner::ResWord* Scanner::rw_table [] =  // reserved word table
{
	NULL, NULL, rw2, rw3, rw4, rw5, rw6, rw7,
};


//=====================================================================================================================================
// ascii map                                                                                                                          =
//=====================================================================================================================================
uint Scanner::asciiLookupTable [128] = {AC_ERROR};

void Scanner::initAsciiMap()
{
	memset( &asciiLookupTable[0], AC_ERROR, sizeof(asciiLookupTable) );
	for( uint x='a'; x<='z'; x++) asciiLookupTable[x] = AC_LETTER;
	for( uint x='A'; x<='Z'; x++) asciiLookupTable[x] = AC_LETTER;
	for( uint x='0'; x<='9'; x++) asciiLookupTable[x] = AC_DIGIT;

	asciiLookupTable[':'] = asciiLookupTable['['] = asciiLookupTable[']'] = asciiLookupTable['('] = AC_SPECIAL;
	asciiLookupTable[')'] = asciiLookupTable['.'] = asciiLookupTable['{'] = asciiLookupTable['}'] = AC_SPECIAL;
	asciiLookupTable[','] = asciiLookupTable[';'] = asciiLookupTable['?'] = asciiLookupTable['='] = AC_SPECIAL;
	asciiLookupTable['!'] = asciiLookupTable['<'] = asciiLookupTable['>'] = asciiLookupTable['|'] = AC_SPECIAL;
	asciiLookupTable['&'] = asciiLookupTable['+'] = asciiLookupTable['-'] = asciiLookupTable['*'] = AC_SPECIAL;
	asciiLookupTable['/'] = asciiLookupTable['~'] = asciiLookupTable['%'] = asciiLookupTable['#'] = AC_SPECIAL;
	asciiLookupTable['^'] = AC_SPECIAL;

	asciiLookupTable['\t'] = asciiLookupTable[' '] = asciiLookupTable['\0'] = asciiLookupTable['\r'] = AC_WHITESPACE;
	asciiLookupTable['\n'] = AC_ERROR; // newline is unacceptable char
	                   
	asciiLookupTable['\"']          = AC_DOUBLEQUOTE;
	asciiLookupTable['\'']          = AC_QUOTE;
	asciiLookupTable[(int)eofChar] = AC_EOF;
	asciiLookupTable['_']           = AC_LETTER;
}


//=====================================================================================================================================
// Scanner                                                                                                                          =
//=====================================================================================================================================
Scanner::Scanner( bool newlinesAsWhitespace_ ):
	newlinesAsWhitespace(newlinesAsWhitespace_), commentedLines(0), inStream(NULL)
{
	if( asciiLookupTable['a'] != AC_LETTER )
		initAsciiMap();

	lineNmbr = 0;
	memset( line, eofChar, sizeof(char)*MAX_SCRIPT_LINE_LEN );
}


//=====================================================================================================================================
// getLine                                                                                                                            =
// it simply gets a new line from the file and it points to the first char of that line                                               =
//=====================================================================================================================================
void Scanner::getLine()
{
	/*if( inStream->eof() )
		pchar = &eofChar;
	else
	{
		inStream->getline( line, MAX_SCRIPT_LINE_LEN - 1 );
		pchar = &line[0];
		++lineNmbr;
	}*/
	if( !inStream->getline( line, MAX_SCRIPT_LINE_LEN - 1, '\n' ) )
		pchar = &eofChar;
	else
	{
		pchar = &line[0];
		++lineNmbr;
	}
	DEBUG_ERR( inStream->gcount() > MAX_SCRIPT_LINE_LEN - 10 ); // too big line
	
	/*if( *pchar != eofChar )
	{
		PRINT( lineNmbr << ": " << line );
	}
	else
	{
		PRINT( lineNmbr << ": eof" );
	}*/
}


//=====================================================================================================================================
// getNextChar                                                                                                                        =
//=====================================================================================================================================
char Scanner::getNextChar()
{
	if( *pchar=='\0' )
		getLine();
	else
		++pchar;

	return *pchar;
}


//=====================================================================================================================================
// putBackChar                                                                                                                        =
//=====================================================================================================================================
char Scanner::putBackChar()
{
	if( pchar != line && *pchar != eofChar )
		--pchar;

	return *pchar;
}


//=====================================================================================================================================
// GetTokenInfo                                                                                                                       =
//=====================================================================================================================================
string Scanner::getTokenInfo( const Token& token )
{
	char token_info_str[256];
	switch( token.code )
	{
		case TC_COMMENT:
			return "comment";
		case TC_NEWLINE:
			return "newline";
		case TC_EOF:
			return "end of file";
		case TC_STRING:
			sprintf( token_info_str, "string \"%s\"", token.value.string );
			break;
		case TC_CHAR:
			sprintf( token_info_str, "char '%c' (\"%s\")", token.value.char_, token.asString );
			break;
		case TC_NUMBER:
			if( token.type == DT_FLOAT )
				sprintf( token_info_str, "float %f or %e (\"%s\")", token.value.float_, token.value.float_, token.asString );
			else
				sprintf( token_info_str, "int %lu (\"%s\")", token.value.int_, token.asString );
			break;
		case TC_IDENTIFIER:
			sprintf( token_info_str, "identifier \"%s\"", token.value.string );
			break;
		case TC_ERROR:
			return "scanner error";
			break;
		default:
			if( token.code>=TC_KE && token.code<=TC_KEYWORD )
				sprintf( token_info_str, "reserved word \"%s\"", token.value.string );
			else if( token.code>=TC_SCOPERESOLUTION && token.code<=TC_ASSIGNOR )
				sprintf( token_info_str, "operator no %d", token.code - TC_SCOPERESOLUTION );
	}

	return string(token_info_str);
}


//=====================================================================================================================================
// printTokenInfo                                                                                                                     =
//=====================================================================================================================================
void Scanner::printTokenInfo( const Token& token )
{
	cout << "Token: " << getTokenInfo(token) << endl;
}


//=====================================================================================================================================
// getAllprintAll                                                                                                                     =
//=====================================================================================================================================
void Scanner::getAllprintAll()
{
	do
	{
		getNextToken();
		cout << setw(3) << setfill('0') << getLineNmbr() << ": " << getTokenInfo( crntToken ) << endl;
	} while( crntToken.code != TC_EOF );
}


//=====================================================================================================================================
// loadFile                                                                                                                           =
//=====================================================================================================================================
bool Scanner::loadFile( const char* filename_ )
{	
	inFstream.open( filename_, ios::in );
	if( !inFstream.good() )
	{
		ERROR( "Cannot open file \"" << filename_ << '\"' );
		return false;
	}
	
	return loadIoStream( &inFstream, filename_ );
}


//=====================================================================================================================================
// loadIoStream                                                                                                                       =
//=====================================================================================================================================
bool Scanner::loadIoStream( iostream* iostream_, const char* scriptName_ )
{
	if( inStream != NULL )
	{
		ERROR( "Tokenizer already initialized" );
		return false;
	}

	inStream = iostream_;

	// init globals
	DEBUG_ERR( strlen(scriptName_) > sizeof(scriptName)/sizeof(char) - 1 ) // Too big name
	crntToken.code = TC_ERROR;
	lineNmbr = 0;
	strcpy( scriptName, scriptName_ );


	getLine();
	return true;
}


//=====================================================================================================================================
// unload                                                                                                                             =
//=====================================================================================================================================
void Scanner::unload()
{
	inFstream.close();
}


//=====================================================================================================================================
// getNextToken                                                                                                                       =
//=====================================================================================================================================
const Scanner::Token& Scanner::getNextToken()
{
	start:

	//if( crntToken.code == TC_NEWLINE ) getNextChar();

	if( commentedLines>0 )
	{
		crntToken.code = TC_NEWLINE;
		--commentedLines;
		++lineNmbr; // the ultimate hack. I should remember not to do such crapp
	}
	else if( *pchar == '/' )
	{
		char ch = getNextChar();
		if( ch == '/' || ch == '*' )
		{
			putBackChar();
			int line = getLineNmbr();
			checkComment();
			
			commentedLines = getLineNmbr() - line; // update commentedLines
			lineNmbr -= commentedLines; // part of the ultimate hack
		}
		else
		{
			putBackChar();
			goto crappy_label;
		}
	}
	else if( *pchar == '.' )
	{
		uint asc = asciiLookup(getNextChar());
		putBackChar();
		if( asc == AC_DIGIT )
			checkNumber();
		else
			checkSpecial();
	}
	else if( *pchar=='\0' ) // if newline
	{
		if( asciiLookup( getNextChar() ) == AC_EOF )
			crntToken.code = TC_EOF;
		else
			crntToken.code = TC_NEWLINE;
	}
	else
	{
		crappy_label:
		switch( asciiLookup(*pchar) )
		{
			case AC_WHITESPACE : getNextChar();  goto start;
			case AC_LETTER     : checkWord();    break;
			case AC_DIGIT      : checkNumber();  break;
			case AC_SPECIAL    : checkSpecial(); break;
			case AC_QUOTE      : checkChar();    break;
			case AC_DOUBLEQUOTE: checkString();  break;
			case AC_EOF:
				crntToken.code = TC_EOF;
				break;
			case AC_ERROR:
			default:
				SERROR( "Unexpected character \'" << *pchar << '\'');

				getNextChar();
				goto start;
		}
	}

	if( crntToken.code == TC_COMMENT ) goto start; // skip comments
	if( crntToken.code == TC_NEWLINE && newlinesAsWhitespace ) goto start;
	
	return crntToken;
}


/*
=======================================================================================================================================
CHECKERS (bellow only checkers)                                                                                                       =
=======================================================================================================================================
*/

//=====================================================================================================================================
// CheckWord                                                                                                                          =
//=====================================================================================================================================
bool Scanner::checkWord()
{
	char* tmp_str = crntToken.asString;
	char ch = *pchar;

	//build the string
	do
	{
		*tmp_str++ = ch;
		ch = getNextChar();
	}while ( asciiLookup(ch)==AC_LETTER || asciiLookup(ch)==AC_DIGIT );

	*tmp_str = '\0'; // finalize it

	//check if reserved
	int len = tmp_str-crntToken.asString;
	crntToken.code = TC_IDENTIFIER;
	crntToken.value.string = crntToken.asString;
	crntToken.type = DT_STR; // not important

	if( len<=7 && len>=2 )
	{
		int x = 0;
		for (;;)
		{
			if( rw_table[len][x].string == NULL ) break;

			if( strcmp(rw_table[len][x].string, crntToken.asString ) == 0 )
			{
				crntToken.code = rw_table[len][x].code;
				break;
			}
			++x;
		}
	}

	return true;
}


//=====================================================================================================================================
// CheckComment                                                                                                                       =
//=====================================================================================================================================
bool Scanner::checkComment()
{
	// begining
	if( getNextChar()=='*' )
		goto branchy_cmnt;
	else if( *pchar=='/' )
	{
		// end
		getLine();
		crntToken.code = TC_COMMENT;
		return true;
	}
	else
		goto error;

	// multi-line comment
	branchy_cmnt:
		if( getNextChar()=='*' )
			goto finalize_branchy;
		else if( *pchar==eofChar )
			goto error;
		else
			goto branchy_cmnt;

	// multi-line "branchy"
	finalize_branchy:
		if( getNextChar()=='/' )
		{
			crntToken.code = TC_COMMENT;
			getNextChar();
			return true;
		}
		else
			goto branchy_cmnt;

	//error
	error:
		crntToken.code = TC_ERROR;
		SERROR( "Incorrect comment ending" );
		return false;
}


//=====================================================================================================================================
// CheckNumber                                                                                                                        =
//=====================================================================================================================================
bool Scanner::checkNumber()
{
	//DEBUG_ERR( sizeof(long) != 8 ); // ulong must be 64bit
	long num = 0;      // value of the number & part of the float num before '.'
	long fnum = 0;     // part of the float num after '.'
	long dad = 0;      // digits after dot (for floats)
	bool exp_sign = 0; // exponent sign in case float is represented in mant/exp format. 0 means positive and 1 negative
	long exp = 0;      // the exponent in case float is represented in mant/exp format
	char* tmp_str = crntToken.asString;
	crntToken.type = DT_INT;
	uint asc;

	// begin
		if( *pchar == '0' )
			goto _0;
		else if( asciiLookup(*pchar) == AC_DIGIT )
		{
			num = num*10 + *pchar-'0';
			goto _0_9;
		}
		else if ( *pchar == '.' )
			goto _float;
		else
			goto error;

	// 0????
	_0:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);
		if ( *pchar == 'x' || *pchar == 'X' )
			goto _0x;
		else if( *pchar == 'e' || *pchar == 'E' )
			goto _0_9_dot_0_9_e;
		else if( asc == AC_DIGIT )
		{
			putBackChar();
			goto _0_9;
		}
		else if( *pchar == '.' )
			goto _float;
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error;

	// 0x????
	_0x:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);
		if( (asc == AC_DIGIT)  ||
				(*pchar >= 'a' && *pchar <= 'f' ) ||
				(*pchar >= 'A' && *pchar <= 'F' ) )
		{
			num <<= 4;
			if( *pchar>='a' && *pchar<='f' )
				num += *pchar - 'a' + 0xA;
			else if( *pchar>='A' && *pchar<='F' )
				num += *pchar - 'A' + 0xA;
			else
				num += *pchar - '0';

			goto _0x0_9orA_F;
		}
		else
			goto error;

	// 0x{0-9 || a-f}??
	_0x0_9orA_F:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);
		if( (asc == AC_DIGIT)         ||
				(*pchar >= 'a' && *pchar <= 'f' ) ||
				(*pchar >= 'A' && *pchar <= 'F' ) )
		{
			num <<= 4;
			if( *pchar>='a' && *pchar<='f' )
				num += *pchar - 'a' + 0xA;
			else if( *pchar>='A' && *pchar<='F' )
				num += *pchar - 'A' + 0xA;
			else
				num += *pchar - '0';

			goto _0x0_9orA_F;
		}
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error; // err

	// {0-9}
	_0_9:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);
		if( asc == AC_DIGIT )
		{
			num = num * 10 + *pchar - '0';
			goto _0_9;
		}
		else if( *pchar == 'e' || *pchar == 'E' )
			goto _0_9_dot_0_9_e;
		else if( *pchar == '.' )
			goto _float;
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error; // err

	// {0-9}.??
	_float:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);
		crntToken.type = DT_FLOAT;
		if( asc == AC_DIGIT )
		{
			fnum = fnum * 10 + *pchar - '0';
			++dad;
			goto _float;
		}
		else if( *pchar == '.' )
		{
			*tmp_str++ = *pchar;
			getNextChar();
			goto error;
		}
		else if( *pchar == 'f' || *pchar == 'F' )
		{
			goto _0_9_dot_0_9_f;
		}
		else if( *pchar == 'e' || *pchar == 'E' )
			goto _0_9_dot_0_9_e;
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error;

	// [{0-9}].[{0-9}]f??
	_0_9_dot_0_9_f:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);

		if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error;

	// [{0-9}].[{0-9}]e??
	_0_9_dot_0_9_e:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);
		crntToken.type = DT_FLOAT;

		if( *pchar == '+' || *pchar == '-' )
		{
			if( *pchar == '-' ) exp_sign = 1;
			//*tmp_str++ = *pchar; getNextChar();
			goto _0_9_dot_0_9_e_sign;
		}
		else if( asc == AC_DIGIT )
		{
			exp = exp * 10 + *pchar - '0';
			goto _0_9_dot_0_9_e_sign_0_9;
		}
		else
			goto error;

	// [{0-9}].[{0-9}]e{+,-}??
	// After the sign we want number
	_0_9_dot_0_9_e_sign:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);

		if( asc == AC_DIGIT )
		{
			exp = exp * 10 + *pchar - '0';
			goto _0_9_dot_0_9_e_sign_0_9;
		}
		else
			goto error;

	// [{0-9}].[{0-9}]e{+,-}{0-9}??
	// After the number in exponent we want other number or we finalize
	_0_9_dot_0_9_e_sign_0_9:
		*tmp_str++ = *pchar;
		getNextChar();
		asc = asciiLookup(*pchar);

		if( asc == AC_DIGIT )
		{
			exp = exp * 10 + *pchar - '0';
			goto _0_9_dot_0_9_e_sign_0_9;
		}
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error;

	// finalize
	finalize:
		crntToken.code = TC_NUMBER;

		if( crntToken.type == DT_INT )
		{
			crntToken.value.int_ = num;
		}
		else
		{
			double dbl = (double)num + (double)(pow(10, -dad)*fnum);
			if( exp != 0 ) // if we have exponent
			{
				if( exp_sign == 1 ) exp = -exp; // change the sign if necessary
				dbl = dbl * pow( 10, exp );
			}

			crntToken.value.float_ = dbl;
		}
		*tmp_str = '\0';
		return true;

	//error
	error:
		crntToken.code = TC_ERROR;

		// run until white space or special
		asc = asciiLookup(*pchar);
		while( asc!=AC_WHITESPACE && asc!=AC_SPECIAL && asc!=AC_EOF )
		{
			*tmp_str++ = *pchar;
			asc = asciiLookup(getNextChar());
		}

		*tmp_str = '\0';
		SERROR( "Bad number suffix \"" << crntToken.asString << '\"' );

	return false;
}


//=====================================================================================================================================
// CheckString                                                                                                                        =
//=====================================================================================================================================
bool Scanner::checkString()
{
	char* tmp_str = crntToken.asString;
	char ch = getNextChar();

	for(;;)
	{
		//Error
		if( ch=='\0' || ch==eofChar ) // if end of line or eof
		{
			crntToken.code = TC_ERROR;
			*tmp_str = '\0';
			SERROR( "Incorect string ending \"" << crntToken.asString );
			return false;
		}
		//Escape Codes
		else if( ch=='\\' )
		{
			ch = getNextChar();
			if( ch=='\0' || ch==eofChar )
			{
				crntToken.code = TC_ERROR;
				*tmp_str = '\0';
				SERROR( "Incorect string ending \"" << crntToken.asString << '\"' );
				return false;
			}

			switch( ch )
			{
				case 'n' : *tmp_str++ = '\n'; break;
				case 't' : *tmp_str++ = '\t'; break;
				case '0' : *tmp_str++ = '\0'; break;
				case 'a' : *tmp_str++ = '\a'; break;
				case '\"': *tmp_str++ = '\"'; break;
				case 'f' : *tmp_str++ = '\f'; break;
				case 'v' : *tmp_str++ = '\v'; break;
				case '\'': *tmp_str++ = '\''; break;
				case '\\': *tmp_str++ = '\\'; break;
				case '\?': *tmp_str++ = '\?'; break;
				default  :
					SERROR( "Unrecognized escape charachter \'\\" << ch << '\'' );
					*tmp_str++ = ch;
			}
		}
		//End
		else if( ch=='\"' )
		{
			*tmp_str = '\0';
			crntToken.code = TC_STRING;
			crntToken.value.string = crntToken.asString;
			getNextChar();
			return true;
		}
		//Build str( main loop )
		else
		{
			*tmp_str++ = ch;
		}

		ch = getNextChar();
	}

	return false;
}


//=====================================================================================================================================
// checkChar                                                                                                                          =
//=====================================================================================================================================
bool Scanner::checkChar()
{
	char ch = getNextChar();
	char ch0 = ch;
	char* tmp_str = crntToken.asString;

	crntToken.code = TC_ERROR;
	*tmp_str++ = ch;

	if( ch=='\0' || ch==eofChar ) // check char after '
	{
		SERROR( "Newline in constant" );
		return false;
	}

	if (ch=='\'') // if '
	{
		SERROR( "Empty constant" );
		getNextChar();
		return false;
	}

	if (ch=='\\')                // if \ then maybe escape char
	{
		ch = getNextChar();
		*tmp_str++ = ch;
		if( ch=='\0' || ch==eofChar ) //check again after the \.
		{
			SERROR( "Newline in constant" );
			return false;
		}

		switch (ch)
		{
			case 'n' : ch0 = '\n'; break;
			case 't' : ch0 = '\t'; break;
			case '0' : ch0 = '\0'; break;
			case 'a' : ch0 = '\a'; break;
			case '\"': ch0 = '\"'; break;
			case 'f' : ch0 = '\f'; break;
			case 'v' : ch0 = '\v'; break;
			case '\'': ch0 = '\''; break;
			case '\\': ch0 = '\\'; break;
			case '\?': ch0 = '\?'; break;
			default  : ch0 = ch  ; SERROR( "Unrecognized escape charachter \'\\" << ch << '\'' );
		}
		crntToken.value.char_ = ch0;
	}
	else
	{
		crntToken.value.char_ = ch;
	}

	ch = getNextChar();
	if( ch=='\'' )    //end
	{
		*tmp_str = '\0';
		crntToken.code = TC_CHAR;
		getNextChar();
		return true;
	}

	SERROR( "Expected \'");
	return false;
}


//=====================================================================================================================================
// checkSpecial                                                                                                                       =
//=====================================================================================================================================
bool Scanner::checkSpecial()
{
	char ch = *pchar;
	TokenCode code = TC_ERROR;

	switch( ch )
	{
		case '#': code = TC_SHARP; break;
		case ',': code = TC_COMMA; break;
		case ';': code = TC_PERIOD; break;
		case '(': code = TC_LPAREN; break;
		case ')': code = TC_RPAREN; break;
		case '[': code = TC_LSQBRACKET; break;
		case ']': code = TC_RSQBRACKET; break;
		case '{': code = TC_LBRACKET; break;
		case '}': code = TC_RBRACKET; break;
		case '?': code = TC_QUESTIONMARK; break;
		case '~': code = TC_ONESCOMPLEMENT; break;

		case '.':
			ch = getNextChar();
			switch( ch )
			{
				case '*':
					code = TC_POINTERTOMEMBER;
					break;
				default:
					putBackChar();
					code = TC_DOT;
			}
			break;

		case ':':
			ch = getNextChar();
			switch( ch )
			{
				case ':':
					code = TC_SCOPERESOLUTION;
					break;
				default:
					putBackChar();
					code = TC_UPDOWNDOT;
			}
			break;

		case '-':
			ch = getNextChar();
			switch( ch )
			{
				case '>':
					code = TC_POINTERTOMEMBER;
					break;
				case '-':
					code = TC_DEC;
					break;
				case '=':
					code = TC_ASSIGNSUB;
					break;
				default:
					putBackChar();
					code = TC_MINUS;
			}
			break;

		case '=':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_EQUAL;
					break;
				default:
					putBackChar();
					code = TC_ASSIGN;
			}
			break;

		case '!':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_NOTEQUAL;
					break;
				default:
					putBackChar();
					code = TC_NOT;
			}
			break;

		case '<':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_LESSEQUAL;
					break;
				case '<':
					ch = getNextChar();
					switch( ch )
					{
						case '=':
							code = TC_ASSIGNSHL;
							break;
						default:
							putBackChar();
							code = TC_SHL;
					}
					break;
				default:
					putBackChar();
					code = TC_LESS;
			}
			break;

		case '>':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_GREATEREQUAL;
					break;
				case '>':
					ch = getNextChar();
					switch( ch )
					{
						case '=':
							code = TC_ASSIGNSHR;
							break;
						default:
							putBackChar();
							code = TC_SHR;
					}
					break;
				default:
					putBackChar();
					code = TC_GREATER;
			}
			break;

		case '|':
			ch = getNextChar();
			switch( ch )
			{
				case '|':
					code = TC_LOGICALOR;
					break;
				case '=':
					code = TC_ASSIGNOR;
					break;
				default:
					putBackChar();
					code = TC_BITWISEOR;
			}
			break;

		case '&':
			ch = getNextChar();
			switch( ch )
			{
				case '&':
					code = TC_LOGICALAND;
					break;
				case '=':
					code = TC_ASSIGNAND;
					break;
				default:
					putBackChar();
					code = TC_BITWISEAND;
			}
			break;

		case '+':
			ch = getNextChar();
			switch( ch )
			{
				case '+':
					code = TC_INC;
					break;
				case '=':
					code = TC_ASSIGNADD;
					break;
				default:
					putBackChar();
					code = TC_PLUS;
			}
			break;

		case '*':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNMUL;
					break;
				default:
					putBackChar();
					code = TC_STAR;
			}
			break;

		case '/':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNDIV;
					break;
				default:
					putBackChar();
					code = TC_BSLASH;
			}
			break;

		case '%':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNMOD;
					break;
				default:
					putBackChar();
					code = TC_MOD;
			}
			break;

		case '^':
			ch = getNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNXOR;
					break;
				default:
					putBackChar();
					code = TC_XOR;
			}
			break;
	}

	getNextChar();
	crntToken.code = code;

	return true;
}
