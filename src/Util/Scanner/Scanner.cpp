#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <cassert>
#include "Scanner.h"
#include "ScannerException.h"
#include <sstream>
#include <iomanip>
#include <boost/lexical_cast.hpp>


namespace Scanner {

#define SCANNER_EXCEPTION(x) \
	Exception(std::string() + x, __LINE__, scriptName, lineNmbr)


//==============================================================================
// statics                                                                     =
//==============================================================================
char Scanner::eofChar = 0x7F;


// reserved words grouped by length
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

Scanner::ResWord* Scanner::rwTable [] =  // reserved word table
{
	NULL, NULL, rw2, rw3, rw4, rw5, rw6, rw7,
};

// ascii table
Scanner::AsciiFlag Scanner::asciiLookupTable [128] = {AC_ERROR};


//==============================================================================
// Constructors                                                                =
//==============================================================================

Scanner::Scanner(bool newlinesAsWhitespace)
{
	strcpy(scriptName, "unnamed-script");
	init(newlinesAsWhitespace);
}


Scanner::Scanner(const char* filename, bool newlinesAsWhitespace)
{
	strcpy(scriptName, "unnamed-script");
	init(newlinesAsWhitespace);
	loadFile(filename);
}


Scanner::Scanner(std::istream& istream_, const char* scriptName_,
	bool newlinesAsWhitespace)
{
	strcpy(scriptName, "unnamed-script");
	init(newlinesAsWhitespace);
	loadIstream(istream_, scriptName_);
}


//==============================================================================
// initAsciiMap                                                                =
//==============================================================================
void Scanner::initAsciiMap()
{
	memset(&asciiLookupTable[0], AC_ERROR, sizeof(asciiLookupTable));
	for(uint x='a'; x<='z'; x++)
	{
		asciiLookupTable[x] = AC_LETTER;
	}

	for(uint x='A'; x<='Z'; x++)
	{
		asciiLookupTable[x] = AC_LETTER;
	}

	for(uint x='0'; x<='9'; x++)
	{
		asciiLookupTable[x] = AC_DIGIT;
	}

	asciiLookupTable[':'] = asciiLookupTable['['] = asciiLookupTable[']'] =
		asciiLookupTable['('] = asciiLookupTable[')'] = asciiLookupTable['.'] =
		asciiLookupTable['{'] = asciiLookupTable['}'] = asciiLookupTable[','] =
		asciiLookupTable[';'] = asciiLookupTable['?'] = asciiLookupTable['='] =
		asciiLookupTable['!'] = asciiLookupTable['<'] = asciiLookupTable['>'] =
		asciiLookupTable['|'] = asciiLookupTable['&'] = asciiLookupTable['+'] =
		asciiLookupTable['-'] = asciiLookupTable['*'] = asciiLookupTable['/'] =
		asciiLookupTable['~'] = asciiLookupTable['%'] = asciiLookupTable['#'] =
		asciiLookupTable['^'] = AC_SPECIAL;

	asciiLookupTable['\t'] = asciiLookupTable[' '] = asciiLookupTable['\0'] =
		AC_WHITESPACE;
	asciiLookupTable['\n'] = AC_ERROR; // newline is unacceptable char

	asciiLookupTable['@'] = asciiLookupTable['`'] = asciiLookupTable['$'] =
		AC_ACCEPTABLE_IN_COMMENTS;

	asciiLookupTable['\"'] = AC_DOUBLEQUOTE;
	asciiLookupTable['\''] = AC_QUOTE;
	asciiLookupTable[(int)eofChar] = AC_EOF;
	asciiLookupTable['_'] = AC_LETTER;
}


//==============================================================================
// init                                                                        =
//==============================================================================
void Scanner::init(bool newlinesAsWhitespace_)
{
	newlinesAsWhitespace = newlinesAsWhitespace_;
	commentedLines = 0;
	inStream = NULL;

	if(asciiLookupTable['a'] != AC_LETTER)
	{
		initAsciiMap();
	}

	lineNmbr = 0;
	memset(line, eofChar, sizeof(char) * MAX_SCRIPT_LINE_LEN);
}


//==============================================================================
// getLine                                                                     =
//==============================================================================
void Scanner::getLine()
{
	if(!inStream->getline(line, MAX_SCRIPT_LINE_LEN - 1, '\n'))
	{
		pchar = &eofChar;
	}
	else
	{
		pchar = &line[0];
		++lineNmbr;
	}

	assert(inStream->gcount() <= MAX_SCRIPT_LINE_LEN - 10); // too big line
}


//==============================================================================
// getNextChar                                                                 =
//==============================================================================
char Scanner::getNextChar()
{
	if(*pchar=='\0')
	{
		getLine();
	}
	else
	{
		++pchar;
	}

	if(*pchar == '\r') // windows crap
	{
		*pchar = '\0';
	}
	else if(lookupAscii(*pchar) == AC_ERROR)
	{
		throw SCANNER_EXCEPTION("Unacceptable char '" + *pchar + "' 0x" +
			boost::lexical_cast<std::string>(static_cast<uint>(*pchar)));
	}

	return *pchar;
}


//==============================================================================
// putBackChar                                                                 =
//==============================================================================
char Scanner::putBackChar()
{
	if(pchar != line && *pchar != eofChar)
	{
		--pchar;
	}

	return *pchar;
}


//==============================================================================
// getAllPrintAll                                                              =
//==============================================================================
void Scanner::getAllPrintAll()
{
	do
	{
		getNextToken();
		std::cout << std::setw(3) << std::setfill('0') << getLineNumber() <<
			": " << crntToken.getInfoStr() << std::endl;
	} while(crntToken.code != TC_EOF);
}


//==============================================================================
// loadFile                                                                    =
//==============================================================================
void Scanner::loadFile(const char* filename_)
{
	inFstream.open(filename_);
	if(!inFstream.is_open())
	{
		throw SCANNER_EXCEPTION("Cannot open file \"" + filename_ + '\"');
	}

	loadIstream(inFstream, filename_);
}


//==============================================================================
// loadIstream                                                                 =
//==============================================================================
void Scanner::loadIstream(std::istream& istream_, const char* scriptName_)
{
	if(inStream != NULL)
	{
		throw SCANNER_EXCEPTION("Tokenizer already initialized");
	}

	inStream = &istream_;

	// init globals

	// Too big name
	assert(strlen(scriptName_) <= sizeof(scriptName) / sizeof(char) - 1);
	crntToken.code = TC_ERROR;
	lineNmbr = 0;
	strcpy(scriptName, scriptName_);

	getLine();
}


//==============================================================================
// unload                                                                      =
//==============================================================================
void Scanner::unload()
{
	inFstream.close();
}


//==============================================================================
// getNextToken                                                                =
//==============================================================================
const Token& Scanner::getNextToken()
{
	start:

	//if(crntToken.code == TC_NEWLINE) getNextChar();

	if(commentedLines>0)
	{
		crntToken.code = TC_NEWLINE;
		--commentedLines;
		// the ultimate hack. I should remember not to do such crap in the
		// future
		++lineNmbr;
	}
	else if(*pchar == '/')
	{
		char ch = getNextChar();
		if(ch == '/' || ch == '*')
		{
			putBackChar();
			int line = getLineNumber();
			checkComment();

			commentedLines = getLineNumber() - line; // update commentedLines
			lineNmbr -= commentedLines; // part of the ultimate hack
		}
		else
		{
			putBackChar();
			goto crappyLabel;
		}
	}
	else if(*pchar == '.')
	{
		uint asc = lookupAscii(getNextChar());
		putBackChar();
		if(asc == AC_DIGIT)
		{
			checkNumber();
		}
		else
		{
			checkSpecial();
		}
	}
	else if(*pchar=='\0') // if newline
	{
		if(lookupAscii(getNextChar()) == AC_EOF)
		{
			crntToken.code = TC_EOF;
		}
		else
		{
			crntToken.code = TC_NEWLINE;
		}
	}
	else
	{
		crappyLabel:
		switch(lookupAscii(*pchar))
		{
			case AC_WHITESPACE :
				getNextChar();
				goto start;
			case AC_LETTER:
				checkWord();
				break;
			case AC_DIGIT:
				checkNumber();
				break;
			case AC_SPECIAL:
				checkSpecial();
				break;
			case AC_QUOTE:
				checkChar();
				break;
			case AC_DOUBLEQUOTE:
				checkString();
				break;
			case AC_EOF:
				crntToken.code = TC_EOF;
				break;
			case AC_ERROR:
			default:
				getNextChar();
				throw SCANNER_EXCEPTION("Unexpected character \'" + *pchar +
					'\'');
				goto start;
		}
	}

	// skip comments
	if(crntToken.code == TC_COMMENT)
	{
		goto start;
	}
	// skip newlines
	if(crntToken.code == TC_NEWLINE && newlinesAsWhitespace)
	{
		goto start;
	}

	return crntToken;
}

//==============================================================================
// checkWord                                                                   =
//==============================================================================
void Scanner::checkWord()
{
	char* tmpStr = &crntToken.asString[0];
	char ch = *pchar;

	//build the string
	do
	{
		*tmpStr++ = ch;
		ch = getNextChar();
	} while(lookupAscii(ch) == AC_LETTER || lookupAscii(ch) == AC_DIGIT);

	*tmpStr = '\0'; // finalize it

	//check if reserved
	int len = tmpStr - &crntToken.asString[0];
	crntToken.code = TC_IDENTIFIER;
	crntToken.value.string = &crntToken.asString[0];
	crntToken.dataType = DT_STR; // not important

	if(len <= 7 && len >= 2)
	{
		int x = 0;
		while(true)
		{
			if(rwTable[len][x].string == NULL)
			{
				break;
			}

			if(strcmp(rwTable[len][x].string, &crntToken.asString[0]) == 0)
			{
				crntToken.code = rwTable[len][x].code;
				break;
			}
			++x;
		}
	}
}


//==============================================================================
// checkComment                                                                =
//==============================================================================
void Scanner::checkComment()
{
	// Beginning
	if(getNextChar()=='*')
	{
		goto cStyleCmnt;
	}
	// C++ style comment
	else if(*pchar=='/')
	{
		while(true)
		{
			char ch = getNextChar();
			if(ch == '\0')
			{
				crntToken.code = TC_COMMENT;
				return;
			}
			else if(ch == '\\')
			{
				if(getNextChar() == '\0')
				{
					getNextChar();
				}
			}
		}
	}
	else
	{
		goto error;
	}

	// C style comment
	cStyleCmnt:
		if(getNextChar()=='*')
		{
			goto finalizeCCmnt;
		}
		else if(*pchar==eofChar)
		{
			goto error;
		}
		else
		{
			goto cStyleCmnt;
		}

	// C++ style comment
	finalizeCCmnt:
		if(getNextChar()=='/')
		{
			crntToken.code = TC_COMMENT;
			getNextChar();
			return;
		}
		else
		{
			goto cStyleCmnt;
		}

	//error
	error:
		crntToken.code = TC_ERROR;
		throw SCANNER_EXCEPTION("Incorrect comment ending");
}


//==============================================================================
// checkNumber                                                                 =
//==============================================================================
void Scanner::checkNumber()
{
	// This func is working great, dont try to understand it and dont even
	// think to try touching it.

	//RASSERT_THROW_EXCEPTION(sizeof(long) != 8); // ulong must be 64bit
	long num = 0;     // value of the number & part of the float num before '.'
	long fnum = 0;    // part of the float num after '.'
	long dad = 0;     // digits after dot (for floats)
	bool expSign = 0; // exponent sign in case float is represented in mant/exp
	                  // format. 0 means positive and 1 negative
	long exp = 0;     // the exponent in case float is represented in mant/exp
	                  // format
	char* tmpStr = &crntToken.asString[0];
	crntToken.dataType = DT_INT;
	uint asc;

	// begin
		if(*pchar == '0')
		{
			goto _0;
		}
		else if(lookupAscii(*pchar) == AC_DIGIT)
		{
			num = num*10 + *pchar-'0';
			goto _0_9;
		}
		else if (*pchar == '.')
		{
			goto _float;
		}
		else
		{
			goto error;
		}

	// 0????
	_0:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);
		if (*pchar == 'x' || *pchar == 'X')
		{
			goto _0x;
		}
		else if(*pchar == 'e' || *pchar == 'E')
		{
			goto _0_9_dot_0_9_e;
		}
		else if(asc == AC_DIGIT)
		{
			putBackChar();
			goto _0_9;
		}
		else if(*pchar == '.')
		{
			goto _float;
		}
		else if(asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF)
		{
			goto finalize;
		}
		else
		{
			goto error;
		}

	// 0x????
	_0x:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);
		if((asc == AC_DIGIT)  ||
				(*pchar >= 'a' && *pchar <= 'f') ||
				(*pchar >= 'A' && *pchar <= 'F'))
		{
			num <<= 4;
			if(*pchar>='a' && *pchar<='f')
			{
				num += *pchar - 'a' + 0xA;
			}
			else if(*pchar>='A' && *pchar<='F')
			{
				num += *pchar - 'A' + 0xA;
			}
			else
			{
				num += *pchar - '0';
			}

			goto _0x0_9orA_F;
		}
		else
			goto error;

	// 0x{0-9 || a-f}??
	_0x0_9orA_F:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);
		if((asc == AC_DIGIT)         ||
				(*pchar >= 'a' && *pchar <= 'f') ||
				(*pchar >= 'A' && *pchar <= 'F'))
		{
			num <<= 4;
			if(*pchar>='a' && *pchar<='f')
			{
				num += *pchar - 'a' + 0xA;
			}
			else if(*pchar>='A' && *pchar<='F')
			{
				num += *pchar - 'A' + 0xA;
			}
			else
			{
				num += *pchar - '0';
			}

			goto _0x0_9orA_F;
		}
		else if(asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF)
		{
			goto finalize;
		}
		else
		{
			goto error; // err
		}

	// {0-9}
	_0_9:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);
		if(asc == AC_DIGIT)
		{
			num = num * 10 + *pchar - '0';
			goto _0_9;
		}
		else if(*pchar == 'e' || *pchar == 'E')
		{
			goto _0_9_dot_0_9_e;
		}
		else if(*pchar == '.')
		{
			goto _float;
		}
		else if(asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF)
		{
			goto finalize;
		}
		else
		{
			goto error; // err
		}

	// {0-9}.??
	_float:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);
		crntToken.dataType = DT_FLOAT;
		if(asc == AC_DIGIT)
		{
			fnum = fnum * 10 + *pchar - '0';
			++dad;
			goto _float;
		}
		else if(*pchar == '.')
		{
			*tmpStr++ = *pchar;
			getNextChar();
			goto error;
		}
		else if(*pchar == 'f' || *pchar == 'F')
		{
			goto _0_9_dot_0_9_f;
		}
		else if(*pchar == 'e' || *pchar == 'E')
		{
			goto _0_9_dot_0_9_e;
		}
		else if(asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF)
		{
			goto finalize;
		}
		else
		{
			goto error;
		}

	// [{0-9}].[{0-9}]f??
	_0_9_dot_0_9_f:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);

		if(asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF)
		{
			goto finalize;
		}
		else
		{
			goto error;
		}

	// [{0-9}].[{0-9}]e??
	_0_9_dot_0_9_e:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);
		crntToken.dataType = DT_FLOAT;

		if(*pchar == '+' || *pchar == '-')
		{
			if(*pchar == '-') expSign = 1;
			//*tmpStr++ = *pchar; getNextChar();
			goto _0_9_dot_0_9_e_sign;
		}
		else if(asc == AC_DIGIT)
		{
			exp = exp * 10 + *pchar - '0';
			goto _0_9_dot_0_9_e_sign_0_9;
		}
		else
		{
			goto error;
		}

	// [{0-9}].[{0-9}]e{+,-}??
	// After the sign we want number
	_0_9_dot_0_9_e_sign:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);

		if(asc == AC_DIGIT)
		{
			exp = exp * 10 + *pchar - '0';
			goto _0_9_dot_0_9_e_sign_0_9;
		}
		else
		{
			goto error;
		}

	// [{0-9}].[{0-9}]e{+,-}{0-9}??
	// After the number in exponent we want other number or we finalize
	_0_9_dot_0_9_e_sign_0_9:
		*tmpStr++ = *pchar;
		getNextChar();
		asc = lookupAscii(*pchar);

		if(asc == AC_DIGIT)
		{
			exp = exp * 10 + *pchar - '0';
			goto _0_9_dot_0_9_e_sign_0_9;
		}
		else if(asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF)
		{
			goto finalize;
		}
		else
		{
			goto error;
		}

	// finalize
	finalize:
		crntToken.code = TC_NUMBER;

		if(crntToken.dataType == DT_INT)
		{
			crntToken.value.int_ = num;
		}
		else
		{
			double dbl = (double)num + (double)(pow(10, -dad)*fnum);
			if(exp != 0) // if we have exponent
			{
				if(expSign == 1) exp = -exp; // change the sign if necessary
				dbl = dbl * pow(10, exp);
			}

			crntToken.value.float_ = dbl;
		}
		*tmpStr = '\0';
		return;

	//error
	error:
		crntToken.code = TC_ERROR;

		// run until white space or special
		asc = lookupAscii(*pchar);
		while(asc!=AC_WHITESPACE && asc!=AC_SPECIAL && asc!=AC_EOF)
		{
			*tmpStr++ = *pchar;
			asc = lookupAscii(getNextChar());
		}

		*tmpStr = '\0';
		throw SCANNER_EXCEPTION("Bad number suffix \"" +
			&crntToken.asString[0] + '\"');
}


//==============================================================================
// checkString                                                                 =
//==============================================================================
void Scanner::checkString()
{
	char* tmpStr = &crntToken.asString[0];
	char ch = getNextChar();

	for(;;)
	{
		// Error
		if(ch == '\0' || ch == eofChar) // if end of line or eof
		{
			crntToken.code = TC_ERROR;
			*tmpStr = '\0';
			throw SCANNER_EXCEPTION("Incorrect string ending \"" +
				&crntToken.asString[0] + '\"');
			return;
		}
		// Escape Codes
		else if(ch == '\\')
		{
			ch = getNextChar();
			if(ch == eofChar)
			{
				crntToken.code = TC_ERROR;
				*tmpStr = '\0';
				throw SCANNER_EXCEPTION("Incorrect string ending \"" +
					&crntToken.asString[0] + '\"');
				return;
			}

			switch(ch)
			{
				case 'n':
					*tmpStr++ = '\n';
					break;
				case 't':
					*tmpStr++ = '\t';
					break;
				case '0':
					*tmpStr++ = '\0';
					break;
				case 'a':
					*tmpStr++ = '\a';
					break;
				case '\"':
					*tmpStr++ = '\"';
					break;
				case 'f':
					*tmpStr++ = '\f';
					break;
				case 'v':
					*tmpStr++ = '\v';
					break;
				case '\'':
					*tmpStr++ = '\'';
					break;
				case '\\':
					*tmpStr++ = '\\';
					break;
				case '\?':
					*tmpStr++ = '\?';
					break;
				case '\0':
					break; // not an escape char but works almost the same
				default:
					throw SCANNER_EXCEPTION(
						"Unrecognized escape character \'\\" + ch + '\'');
					*tmpStr++ = ch;
			}
		}
		// End
		else if(ch=='\"')
		{
			*tmpStr = '\0';
			crntToken.code = TC_STRING;
			crntToken.value.string = &crntToken.asString[0];
			getNextChar();
			return;
		}
		// Build str(main loop)
		else
		{
			*tmpStr++ = ch;
		}

		ch = getNextChar();
	}
}


//==============================================================================
// checkChar                                                                   =
//==============================================================================
void Scanner::checkChar()
{
	char ch = getNextChar();
	char ch0 = ch;
	char* tmpStr = &crntToken.asString[0];

	crntToken.code = TC_ERROR;
	*tmpStr++ = ch;

	if(ch=='\0' || ch==eofChar) // check char after '
	{
		throw SCANNER_EXCEPTION("Newline in constant");
		return;
	}

	if (ch=='\'') // if '
	{
		throw SCANNER_EXCEPTION("Empty constant");
		getNextChar();
		return;
	}

	if (ch=='\\')                // if \ then maybe escape char
	{
		ch = getNextChar();
		*tmpStr++ = ch;
		if(ch=='\0' || ch==eofChar) //check again after the \.
		{
			throw SCANNER_EXCEPTION("Newline in constant");
			return;
		}

		switch (ch)
		{
			case 'n' :
				ch0 = '\n';
				break;
			case 't' :
				ch0 = '\t';
				break;
			case '0':
				ch0 = '\0';
				break;
			case 'a':
				ch0 = '\a';
				break;
			case '\"':
				ch0 = '\"';
				break;
			case 'f':
				ch0 = '\f';
				break;
			case 'v':
				ch0 = '\v';
				break;
			case '\'':
				ch0 = '\'';
				break;
			case '\\':
				ch0 = '\\';
				break;
			case '\?':
				ch0 = '\?';
				break;
			default:
				ch0 = ch;
				throw SCANNER_EXCEPTION("Unrecognized escape character \'\\" +
					ch + '\'');
		}
		crntToken.value.char_ = ch0;
	}
	else
	{
		crntToken.value.char_ = ch;
	}

	ch = getNextChar();
	if(ch=='\'')    //end
	{
		*tmpStr = '\0';
		crntToken.code = TC_CHAR;
		getNextChar();
		return;
	}

	throw SCANNER_EXCEPTION("Expected \'");
}


//==============================================================================
// checkSpecial                                                                =
//==============================================================================
void Scanner::checkSpecial()
{
	char ch = *pchar;
	TokenCode code = TC_ERROR;

	switch(ch)
	{
		case '#':
			code = TC_SHARP;
			break;
		case ',':
			code = TC_COMMA;
			break;
		case ';':
			code = TC_PERIOD;
			break;
		case '(':
			code = TC_LPAREN;
			break;
		case ')':
			code = TC_RPAREN;
			break;
		case '[':
			code = TC_LSQBRACKET;
			break;
		case ']':
			code = TC_RSQBRACKET;
			break;
		case '{':
			code = TC_LBRACKET;
			break;
		case '}':
			code = TC_RBRACKET;
			break;
		case '?':
			code = TC_QUESTIONMARK;
			break;
		case '~':
			code = TC_ONESCOMPLEMENT;
			break;
		case '.':
			ch = getNextChar();
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
			{
				case '=':
					code = TC_LESSEQUAL;
					break;
				case '<':
					ch = getNextChar();
					switch(ch)
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
			switch(ch)
			{
				case '=':
					code = TC_GREATEREQUAL;
					break;
				case '>':
					ch = getNextChar();
					switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
			switch(ch)
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
}


} // end namesapce
