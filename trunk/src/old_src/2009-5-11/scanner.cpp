#include "scanner.h"
#include <fstream>

namespace scn {

#define SERROR(x) ERROR( "SCAN_ERR (" << filename << ':' << line_nmbr << "): " << x )

/*
=======================================================================================================================================
data vars                                                                                                                             =
=======================================================================================================================================
*/
static char eof_char = 0x7F;

       token_t crnt_token;
static char    line_txt [MAX_SCRIPT_LINE_LEN] = {eof_char};
static char*   pchar;
static fstream file;
static char    token_info_str [128];
       char    filename[64];
       int     line_nmbr = 0;


/*
=======================================================================================================================================
reserved words listed in length                                                                                                       =
=======================================================================================================================================
*/
struct res_word_t
{
	const char*  string;
	token_code_e code;
};

static res_word_t rw2 [] =
{
	{"ke", TC_KE}, {NULL, TC_ERROR}
};

static res_word_t rw3 [] =
{
	{"key", TC_KEY}, {NULL, TC_ERROR}
};

static res_word_t rw4 [] =
{
	{"keyw", TC_KEYW}, {NULL, TC_ERROR}
};

static res_word_t rw5 [] =
{
	{"keywo", TC_KEYWO}, {NULL, TC_ERROR}
};

static res_word_t rw6 [] =
{
	{"keywor", TC_KEYWOR}, {NULL, TC_ERROR}
};

static res_word_t rw7 [] =
{
	{"keyword", TC_KEYWORD}, {NULL, TC_ERROR}
};

static res_word_t* rw_table [] =  //reserved word table
{
	NULL, NULL, rw2, rw3, rw4, rw5, rw6, rw7,
};


/*
=======================================================================================================================================
ascii map                                                                                                                             =
=======================================================================================================================================
*/
enum ascii_code_e
{
	AC_ERROR,
	AC_EOF,
	AC_LETTER,
	AC_DIGIT,
	AC_SPECIAL,
	AC_WHITESPACE,
	AC_QUOTE,
	AC_DOUBLEQUOTE
};

static ascii_code_e ascii_lookup_table [128];

static void InitAsciiMap()
{
	uint x;
	memset( &ascii_lookup_table[0], AC_ERROR, sizeof(char)*128 );
	for (x='a'; x<='z'; x++) ascii_lookup_table[x] = AC_LETTER;
	for (x='A'; x<='Z'; x++) ascii_lookup_table[x] = AC_LETTER;
	for (x='0'; x<='9'; x++) ascii_lookup_table[x] = AC_DIGIT;

	ascii_lookup_table[':'] = ascii_lookup_table['['] = ascii_lookup_table[']'] = ascii_lookup_table['('] = AC_SPECIAL;
	ascii_lookup_table[')'] = ascii_lookup_table['.'] = ascii_lookup_table['{'] = ascii_lookup_table['}'] = AC_SPECIAL;
	ascii_lookup_table[','] = ascii_lookup_table[';'] = ascii_lookup_table['?'] = ascii_lookup_table['='] = AC_SPECIAL;
	ascii_lookup_table['!'] = ascii_lookup_table['<'] = ascii_lookup_table['>'] = ascii_lookup_table['|'] = AC_SPECIAL;
	ascii_lookup_table['&'] = ascii_lookup_table['+'] = ascii_lookup_table['-'] = ascii_lookup_table['*'] = AC_SPECIAL;
	ascii_lookup_table['/'] = ascii_lookup_table['~'] = ascii_lookup_table['%']                  = AC_SPECIAL;

	ascii_lookup_table['\n']= ascii_lookup_table['\t']= ascii_lookup_table['\0']= ascii_lookup_table[' '] = AC_WHITESPACE;

	ascii_lookup_table['\"']          = AC_DOUBLEQUOTE;
	ascii_lookup_table['\'']          = AC_QUOTE;
	ascii_lookup_table[(int)eof_char] = AC_EOF;
	ascii_lookup_table['_']           = AC_LETTER;
}

inline ascii_code_e AsciiLookup( char ch_ )
{
	return ascii_lookup_table[ (int)ch_ ];
}


/*
=======================================================================================================================================
GetLine                                                                                                                               =
it simply gets a new line from the file and it points to the first char of that line                                                  =
=======================================================================================================================================
*/
static void GetLine()
{
	if( file.eof() )
		pchar = &eof_char;
	else
	{
		file.getline( line_txt, MAX_SCRIPT_LINE_LEN - 1 );
		pchar = &line_txt[0];
	}
	++line_nmbr;
}


/*
=======================================================================================================================================
GetNextChar                                                                                                                           =
get the next char from the line_txt. If line_txt empty then get new line. It returns '\0' if we are in the end of the line            =
=======================================================================================================================================
*/
static char GetNextChar()
{
	if( *pchar=='\0' )
		GetLine();
	else
		++pchar;

	return *pchar;
}


/*
=======================================================================================================================================
PutBackChar                                                                                                                           =
=======================================================================================================================================
*/
static char PutBackChar()
{
	if( pchar != line_txt && *pchar != eof_char )
		--pchar;

	return *pchar;
}


/**
=======================================================================================================================================
CHECKERS                                                                                                                              =
=======================================================================================================================================
*/

/*
=======================================================================================================================================
CheckWord                                                                                                                             =
=======================================================================================================================================
*/
bool CheckWord()
{
	char* tmp_str = crnt_token.as_string;
	char ch = *pchar;

	//build the string
	do
	{
		*tmp_str++ = ch;
		ch = GetNextChar();
	}while ( AsciiLookup(ch)==AC_LETTER || AsciiLookup(ch)==AC_DIGIT );

	*tmp_str = '\0';

	//check if reserved
	tmp_str = crnt_token.as_string;
	int len = strlen(tmp_str);
	crnt_token.code = TC_IDENTIFIER;
	crnt_token.value.string = tmp_str;
	crnt_token.type = DT_STR;

	if( len<=7 && len>=2 )
	{
		int x = 0;
		for (;;)
		{
			if( rw_table[len][x].string == NULL ) break;

			if( strcmp(rw_table[len][x].string, tmp_str ) == 0 )
			{
				crnt_token.code = rw_table[len][x].code;
				break;
			}
			++x;
		}
	}

	return true;
}


/*
=======================================================================================================================================
CheckComment                                                                                                                          =
=======================================================================================================================================
*/
bool CheckComment()
{
	// begining
		if( GetNextChar()=='*' )
			goto branchy_cmnt;
		else if( *pchar=='/' )
		{
			// end
			GetLine();
			crnt_token.code = TC_COMMENT;
			return true;
		}
		else
			goto error;

	// "branchy" comment
	branchy_cmnt:
		if( GetNextChar()=='*' )
			goto finalize_branchy;
		else if( *pchar==eof_char )
			goto error;
		else
			goto branchy_cmnt;

	// finalize "branchy"
	finalize_branchy:
		if( GetNextChar()=='/' )
		{
			crnt_token.code = TC_COMMENT;
			GetNextChar();
			return true;
		}
		else
			goto branchy_cmnt;

	//error
	error:
		crnt_token.code = TC_ERROR;
		SERROR( "Incorrect commend ending" );
		return false;
}


/*
=======================================================================================================================================
CheckNumber                                                                                                                           =
=======================================================================================================================================
*/
static bool CheckNumber()
{
	int num = 0;     //value of the number & part of the float num before '.'
	int fnum = 0;    //part of the float num after '.'
	int dad = 0;     //digits after dot (for floats)
	char* tmp_str = crnt_token.as_string;
	crnt_token.type = DT_INT;
	ascii_code_e asc;

	// begin
		if( *pchar == '0' )
			goto _0;
		else if( AsciiLookup(*pchar) == AC_DIGIT )
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
		*tmp_str++ = *pchar; GetNextChar();
		asc = AsciiLookup(*pchar);
		if ( *pchar == 'x' || *pchar == 'X' )
			goto _0x;
		else if( asc == AC_DIGIT )
		{
			PutBackChar();
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
		*tmp_str++ = *pchar; GetNextChar();
		asc = AsciiLookup(*pchar);
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
		*tmp_str++ = *pchar; GetNextChar();
		asc = AsciiLookup(*pchar);
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
		*tmp_str++ = *pchar; GetNextChar();
		asc = AsciiLookup(*pchar);
		if( asc == AC_DIGIT )
		{
			num = num * 10 + *pchar - '0';
			goto _0_9;
		}
		else if( *pchar == '.' )
			goto _float;
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error; // err

	// {0-9}.??
	_float:
		*tmp_str++ = *pchar; GetNextChar();
		asc = AsciiLookup(*pchar);
		crnt_token.type = DT_FLOAT;
		if( asc == AC_DIGIT )
		{
			fnum = fnum * 10 + *pchar - '0';
			++dad;
			goto _float;
		}
		else if( *pchar == '.' )
		{
			*tmp_str++ = *pchar; GetNextChar();
			goto error;
		}
		else if( *pchar == 'f' )
		{
			*tmp_str++ = *pchar; GetNextChar();
			goto finalize;
		}
		else if( asc == AC_SPECIAL || asc == AC_WHITESPACE || asc == AC_EOF )
			goto finalize;
		else
			goto error;

	// finalize
	finalize:
		crnt_token.code = TC_NUMBER;

		if( crnt_token.type == DT_INT )
		{
			crnt_token.value.int_ = num;
		}
		else
		{
			double dbl = (double)num + (double)(pow(10, -dad)*fnum);
			crnt_token.value.float_ = dbl;
		}
		*tmp_str = '\0';
		return true;

	//error
	error:
		crnt_token.code = TC_ERROR;

		// run until white space or special
		asc = AsciiLookup(*pchar);
		while( asc!=AC_WHITESPACE && asc!=AC_SPECIAL && asc!=AC_EOF )
		{
			*tmp_str++ = *pchar;
			asc = AsciiLookup(GetNextChar());
		}

		*tmp_str = '\0';
		SERROR( "Bad number suffix \"" << crnt_token.as_string << '\"' );

	return false;
}


/*
=======================================================================================================================================
CheckString                                                                                                                           =
=======================================================================================================================================
*/
static bool CheckString()
{
	char* tmp_str = crnt_token.as_string;
	char ch = GetNextChar();

	for(;;)
	{
		//Error
		if( ch=='\0' || ch==eof_char ) // if end of line or eof
		{
			crnt_token.code = TC_ERROR;
			*tmp_str = '\0';
			SERROR( "Incorect string ending \"" << crnt_token.as_string );
			return false;
		}
		//Escape Codes
		else if( ch=='\\' )
		{
			ch = GetNextChar();
			if( ch=='\0' || ch==eof_char )
			{
				crnt_token.code = TC_ERROR;
				*tmp_str = '\0';
				SERROR( "Incorect string ending \"" << crnt_token.as_string << '\"' );
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
			crnt_token.code = TC_STRING;
			crnt_token.value.string = crnt_token.as_string;
			GetNextChar();
			return true;
		}
		//Build str( main loop )
		else
		{
			*tmp_str++ = ch;
		}

		ch = GetNextChar();
	}

	return false;
}


/*
=======================================================================================================================================
CheckChar                                                                                                                             =
=======================================================================================================================================
*/
static bool CheckChar()
{
	char ch = GetNextChar();
	char ch0 = ch;
	char* tmp_str = crnt_token.as_string;

	crnt_token.code = TC_ERROR;
	*tmp_str++ = ch;

	if( ch=='\0' || ch==eof_char ) // check char after '
	{
		SERROR( "Newline in constant" );
		return false;
	}

	if (ch=='\'') // if '
	{
		SERROR( "Empty constant" );
		GetNextChar();
		return false;
	}

	if (ch=='\\')                // if \ then maybe escape char
	{
		ch = GetNextChar();
		*tmp_str++ = ch;
		if( ch=='\0' || ch==eof_char ) //check again after the \.
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
		crnt_token.value.char_ = ch0;
	}
	else
	{
		crnt_token.value.char_ = ch;
	}

	ch = GetNextChar();
	if( ch=='\'' )    //end
	{
		*tmp_str = '\0';
		crnt_token.code = TC_CHAR;
		GetNextChar();
		return true;
	}

	SERROR( "Expected \'");
	return false;
}


/*
=======================================================================================================================================
CheckSpecial                                                                                                                          =
=======================================================================================================================================
*/
static bool CheckSpecial()
{
	char ch = *pchar;
	token_code_e code = TC_ERROR;

	switch( ch )
	{
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
			ch = GetNextChar();
			switch( ch )
			{
				case '*':
					code = TC_POINTERTOMEMBER;
					break;
				default:
					PutBackChar();
					code = TC_DOT;
			}
			break;

		case ':':
			ch = GetNextChar();
			switch( ch )
			{
				case ':':
					code = TC_SCOPERESOLUTION;
					break;
				default:
					PutBackChar();
					code = TC_UPDOWNDOT;
			}
			break;

		case '-':
			ch = GetNextChar();
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
					PutBackChar();
					code = TC_MINUS;
			}
			break;

		case '=':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_EQUAL;
					break;
				default:
					PutBackChar();
					code = TC_ASSIGN;
			}
			break;

		case '!':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_NOTEQUAL;
					break;
				default:
					PutBackChar();
					code = TC_NOT;
			}
			break;

		case '<':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_LESSEQUAL;
					break;
				case '<':
					ch = GetNextChar();
					switch( ch )
					{
						case '=':
							code = TC_ASSIGNSHL;
							break;
						default:
							PutBackChar();
							code = TC_SHL;
					}
					break;
				default:
					PutBackChar();
					code = TC_LESS;
			}
			break;

		case '>':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_GREATEREQUAL;
					break;
				case '>':
					ch = GetNextChar();
					switch( ch )
					{
						case '=':
							code = TC_ASSIGNSHR;
							break;
						default:
							PutBackChar();
							code = TC_SHR;
					}
					break;
				default:
					PutBackChar();
					code = TC_GREATER;
			}
			break;

		case '|':
			ch = GetNextChar();
			switch( ch )
			{
				case '|':
					code = TC_LOGICALOR;
					break;
				case '=':
					code = TC_ASSIGNOR;
					break;
				default:
					PutBackChar();
					code = TC_BITWISEOR;
			}
			break;

		case '&':
			ch = GetNextChar();
			switch( ch )
			{
				case '&':
					code = TC_LOGICALAND;
					break;
				case '=':
					code = TC_ASSIGNAND;
					break;
				default:
					PutBackChar();
					code = TC_BITWISEAND;
			}
			break;

		case '+':
			ch = GetNextChar();
			switch( ch )
			{
				case '+':
					code = TC_INC;
					break;
				case '=':
					code = TC_ASSIGNADD;
					break;
				default:
					PutBackChar();
					code = TC_PLUS;
			}
			break;

		case '*':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNMUL;
					break;
				default:
					PutBackChar();
					code = TC_STAR;
			}
			break;

		case '/':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNDIV;
					break;
				default:
					PutBackChar();
					code = TC_BSLASH;
			}
			break;

		case '%':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNMOD;
					break;
				default:
					PutBackChar();
					code = TC_MOD;
			}
			break;

		case '^':
			ch = GetNextChar();
			switch( ch )
			{
				case '=':
					code = TC_ASSIGNXOR;
					break;
				default:
					PutBackChar();
					code = TC_XOR;
			}
			break;
	}

	GetNextChar();
	crnt_token.code = code;

	return true;
}


/**
=======================================================================================================================================
THE EXTERNS                                                                                                                           =
=======================================================================================================================================
*/

/*
=======================================================================================================================================
GetTokenInfo                                                                                                                          =
=======================================================================================================================================
*/
const char* GetTokenInfo( const token_t& token )
{
	switch( token.code )
	{
		case TC_COMMENT:
			strcpy( token_info_str, "comment" );
			break;
		case TC_EOF:
			strcpy( token_info_str, "end of file" );
			break;
		case TC_STRING:
			sprintf( token_info_str, "string \"%s\"", token.value.string );
			break;
		case TC_CHAR:
			sprintf( token_info_str, "char '%c' (\"%s\")", crnt_token.value.char_, token.as_string );
			break;
		case TC_NUMBER:
			if( token.type == DT_FLOAT )
				sprintf( token_info_str, "float %f (\"%s\")", token.value.float_, token.as_string );
			else
				sprintf( token_info_str, "int %lu (\"%s\")", token.value.int_, token.as_string );
			break;
		case TC_IDENTIFIER:
			sprintf( token_info_str, "identifier \"%s\"", token.value.string );
			break;
		case TC_ERROR:
			strcpy( token_info_str, "scanner error" );
			break;
		default:
			if( token.code>=TC_KE && token.code<=TC_KEYWORD )
				sprintf( token_info_str, "reserved word \"%s\"", token.value.string );
			else if( token.code>=TC_SCOPERESOLUTION && token.code<=TC_ASSIGNOR )
				sprintf( token_info_str, "operator no %d", token.code - TC_SCOPERESOLUTION );
	}

	return token_info_str;
}

/*
=======================================================================================================================================
PrintTokenInfo                                                                                                                        =
=======================================================================================================================================
*/
void PrintTokenInfo( const token_t& token )
{
	cout << "Token: " << GetTokenInfo(token) << endl;
}


/*
=======================================================================================================================================
LoadFile                                                                                                                              =
=======================================================================================================================================
*/
bool LoadFile( const char* filename_ )
{
	file.open( filename_, ios::in );

	if( !file.good() )
	{
		ERROR( "Cannot open file \"" << filename_ << '\"' );
		return false;
	}

	strcpy( filename, filename_ );

	// init globals
	pchar = line_txt;
	line_txt[0] = '\0';
	crnt_token.code = TC_ERROR;
	line_nmbr = 0;
	// end init globals

	GetLine();
	return true;
}

/*
=======================================================================================================================================
UnloadFile                                                                                                                            =
=======================================================================================================================================
*/
void UnloadFile()
{
	file.close();
}


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	InitAsciiMap();
}


/*
=======================================================================================================================================
GetNextToken                                                                                                                          =
=======================================================================================================================================
*/
const token_t& GetNextToken()
{
	start:

	if( *pchar == '/' )
	{
		char ch = GetNextChar();
		if( ch == '/' || ch == '*' )
		{
			PutBackChar();
			CheckComment();
		}
		else
		{
			PutBackChar();
			goto crappy_label;
		}
	}
	else if( *pchar == '.' )
	{
		ascii_code_e asc = AsciiLookup(GetNextChar());
		PutBackChar();
		if( asc == AC_DIGIT )
			CheckNumber();
		else
			CheckSpecial();
	}
	else
	{
		crappy_label:
		switch( AsciiLookup(*pchar) )
		{
			case AC_WHITESPACE : GetNextChar();  goto start;
			case AC_LETTER     : CheckWord();    break;
			case AC_DIGIT      : CheckNumber();  break;
			case AC_SPECIAL    : CheckSpecial(); break;
			case AC_QUOTE      : CheckChar();    break;
			case AC_DOUBLEQUOTE: CheckString();  break;
			case AC_EOF:
				crnt_token.code = TC_EOF;
				break;
			case AC_ERROR:
			default:
				SERROR( "Unexpected character \'" << *pchar << '\'');

				GetNextChar();
				goto start;
		}
	}

	if( crnt_token.code == TC_COMMENT ) goto start;
	return crnt_token;
}


} // end namespace
