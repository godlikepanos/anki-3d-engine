#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "common.h"

namespace scn {

/*
=======================================================================================================================================
parser macros                                                                                                                         =
=======================================================================================================================================
*/
#define PARSE_ERR(x) ERROR( "PARSE_ERR (" << scn::GetFilename() << ':' << scn::GetLineNmbr() << "): " << x )

// common parser errors
#define PARSE_ERR_EXPECTED(x) PARSE_ERR( "Expected " << x << " and not " << scn::GetTokenInfo(scn::GetCrntToken()) );
#define PARSE_ERR_UNEXPECTED() PARSE_ERR( "Unexpected token " << scn::GetTokenInfo(scn::GetCrntToken()) );


/*
=======================================================================================================================================
scanner types                                                                                                                         =
=======================================================================================================================================
*/
const int MAX_SCRIPT_LINE_LEN = 256;

// token_code_e
enum token_code_e
{
	// general codes
	TC_ERROR, TC_EOF, TC_COMMENT, TC_NUMBER, TC_CHAR, TC_STRING, TC_IDENTIFIER,

	// keywords listed by strlen (dummy keywords at the moment)
	TC_KE,
	TC_KEY,
	TC_KEYW,
	TC_KEYWO,
	TC_KEYWOR,
	TC_KEYWORD,

	// operators
	TC_SCOPERESOLUTION, TC_LSQBRACKET, TC_RSQBRACKET, TC_LPAREN, TC_RPAREN, TC_DOT, TC_POINTERTOMEMBER, TC_LBRACKET, TC_RBRACKET, TC_COMMA,
		TC_PERIOD, TC_UPDOWNDOT, TC_QUESTIONMARK,                                                                  //0 - 12
	TC_EQUAL, TC_NOTEQUAL, TC_LESS, TC_GREATER, TC_LESSEQUAL, TC_GREATEREQUAL, TC_LOGICALOR, TC_LOGICALAND,            //13 - 20
	TC_PLUS, TC_MINUS, TC_STAR, TC_BSLASH, TC_NOT, TC_BITWISEAND, TC_BITWISEOR, TC_ONESCOMPLEMENT, TC_MOD, TC_XOR,       //21 - 30
	TC_INC, TC_DEC, TC_SHL, TC_SHR,                                                                                //31 - 34
	TC_ASSIGN, TC_ASSIGNADD, TC_ASSIGNSUB, TC_ASSIGNMUL, TC_ASSIGNDIV, TC_ASSIGNMOD, TC_ASSIGNSHL, TC_ASSIGNSHR, TC_ASSIGNAND, TC_ASSIGNXOR,
		TC_ASSIGNOR                                                                                              //35 - 45
};

// token_data_type_e
enum token_data_type_e
{
	DT_FLOAT,
	DT_INT,
	DT_CHAR,
	DT_STR
};

// token_data_val_u
union token_data_val_u
{
	char   char_;
	ulong  int_;
	double float_;
	char*  string; // points to token_t::as_string if the token is string
};

// token_t
class token_t
{
	public:
		token_code_e       code;
		token_data_type_e  type;
		token_data_val_u   value;

		char as_string[ MAX_SCRIPT_LINE_LEN ];

		token_t(): code( TC_ERROR ) {}
};


/*
=======================================================================================================================================
scanner externs                                                                                                                       =
=======================================================================================================================================
*/

extern void Init();
extern bool LoadFile( const char* filename );
extern void UnloadFile();

extern void        PrintTokenInfo( const token_t& token );
extern const char* GetTokenInfo( const token_t& token );

extern const token_t& GetNextToken();
inline const token_t& GetCrntToken() { extern token_t crnt_token; return crnt_token; }

inline const char* GetFilename() { extern char filename[]; return filename; }
inline int         GetLineNmbr() { extern int line_nmbr; return line_nmbr; }

} // end namespace

#endif
