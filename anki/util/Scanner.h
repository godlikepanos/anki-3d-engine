#ifndef ANKI_UTIL_SCANNER_H
#define ANKI_UTIL_SCANNER_H

#include <exception>
#include <boost/array.hpp>
#include <iosfwd>
#include <fstream>


namespace anki { namespace scanner {


/// Scanner exception
class Exception: public std::exception
{
public:
	/// Constructor
	Exception(const std::string& err, int errNo,
		const std::string& scriptFilename, int scriptLineNmbr);

	/// Copy constructor
	Exception(const Exception& e);

	/// Destructor. Do nothing
	~Exception() throw()
	{}

	/// Return the error code
	virtual const char* what() const throw();

private:
	std::string error;
	int errNo; ///< Error number
	std::string scriptFilename;
	int scriptLineNmbr;
	mutable std::string errWhat;
};


/// The max allowed length of a script line
const int MAX_SCRIPT_LINE_LEN = 1024;


/// The TokenCode is an enum that defines the Token type
enum TokenCode
{
	// general codes
	TC_ERROR, TC_END, TC_COMMENT, TC_NUMBER, TC_CHARACTER, TC_STRING,
	TC_IDENTIFIER, TC_NEWLINE,

	// keywords listed by strlen (dummy keywords at the moment)
	TC_KE,
	TC_KEY,
	TC_KEYW,
	TC_KEYWO,
	TC_KEYWOR,
	TC_KEYWORD,

	// operators
	TC_SCOPE_RESOLUTION, TC_L_SQ_BRACKET, TC_R_SQ_BRACKET, TC_L_PAREN,
		TC_R_PAREN,
	TC_DOT, TC_POINTER_TO_MEMBER, TC_L_BRACKET, TC_R_BRACKET, TC_COMMA,
	TC_PERIOD, TC_UPDOWNDOT, TC_QUESTIONMARK, TC_SHARP, TC_EQUAL,
	TC_NOT_EQUAL, TC_LESS, TC_GREATER, TC_LESS_EQUAL, TC_GREATER_EQUAL,
	TC_LOGICAL_OR, TC_LOGICAL_AND, TC_PLUS, TC_MINUS, TC_STAR,
	TC_BSLASH, TC_NOT, TC_BITWISE_AND, TC_BITWISE_OR, TC_UNARAY_COMPLEMENT,
	TC_MOD, TC_XOR, TC_INC, TC_DEC, TC_SHL,
	TC_SHR, TC_ASSIGN, TC_ASSIGN_ADD, TC_ASSIGN_SUB, TC_ASSIGN_MUL,
	TC_ASSIGN_DIV, TC_ASSIGN_MOD, TC_ASSIGN_SHL, TC_ASSIGN_SHR, TC_ASSIGN_AND,
	TC_ASSIGN_XOR, TC_ASSIGN_OR, TC_BACK_SLASH
}; // end enum TokenCode


/// The value of Token::dataType
enum DataType
{
	DT_FLOAT,
	DT_INT,
	DT_CHAR,
	DT_STR
};


/// Used inside the Token, its a variant that holds the data of the Token
class TokenDataVal
{
	friend class Scanner;
	friend class Token;

public:
	/// @name Accessors
	/// @{

	/// Access the data as C char
	char getChar() const
	{
		return char_;
	}

	/// Access the data as unsigned int
	unsigned long getInt() const
	{
		return int_;
	}

	/// Access the data as double
	double getFloat() const
	{
		return float_;
	}

	/// Access the data as C string
	const char* getString() const
	{
		return string;
	}
	/// @}

private:
	/// The data as unnamed union
	union
	{
		char char_;
		unsigned long int_;
		double float_;
		/// Points to @ref Token::asString if the token is string or
		/// identifier
		char* string;
	};
};


/// The Token class
class Token
{
	friend class Scanner;

public:
	Token()
		: code(TC_ERROR)
	{}

	Token(const Token& b);

	/// @name accessors
	/// @{
	const char* getString() const
	{
		return &asString[0];
	}

	TokenCode getCode() const
	{
		return code;
	}

	DataType getDataType() const
	{
		return dataType;
	}

	const TokenDataVal& getValue() const
	{
		return value;
	}
	/// @}

	std::string getInfoString() const;

	friend std::ostream& operator<<(std::ostream& s,
		const Token& x);

private:
	boost::array<char, MAX_SCRIPT_LINE_LEN> asString;
	TokenCode code; ///< The first thing you should know about a token

	/// Additional info in case @ref code is @ref TC_NUMBER
	DataType dataType;
	TokenDataVal value; ///< A value variant
};


/// C++ Tokenizer
///
/// The Scanner loads a file or an already loaded iostream and extracts the
/// tokens. The script must be in C++ format. The class does not make any kind
/// of memory allocations so it can be fast.
class Scanner
{
public:
	/// Constructor #1
	/// @param newlinesAsWhitespace @see newlinesAsWhitespace
	Scanner(bool newlinesAsWhitespace = true);

	/// Constructor #2
	/// @see loadFile
	/// @param newlinesAsWhitespace @see newlinesAsWhitespace
	/// @exception Exception
	Scanner(const char* filename, bool newlinesAsWhitespace = true);

	/// Constructor #3
	/// @see loadIstream
	/// @param newlinesAsWhitespace @see newlinesAsWhitespace
	/// @exception Exception
	Scanner(std::istream& istream_,
		const char* scriptName_ = "unamed-istream",
		bool newlinesAsWhitespace = true);

	/// It only unloads the file if file is chosen
	~Scanner()
	{
		unload();
	}

	/// Load a file to extract tokens
	/// @param filename The filename of the file to read
	/// @exception Exception
	void loadFile(const char* filename);

	/// Load a STL istream to extract tokens
	/// @param istream_ The stream from where to read
	/// @param scriptName_ The name of the stream. For error reporting
	/// @exception Exception
	void loadIstream(std::istream& istream_,
		const char* scriptName_ = "unamed-istream");

	/// Extracts all tokens and prints them. Used for debugging
	void getAllPrintAll();

	/// Get the next token from the stream. Its virtual and you can
	/// override it
	/// @return The next Token
	/// @exception Exception
	virtual const Token& getNextToken();

	/// Accessor for the current token
	/// @return The current Token
	const Token& getCrntToken() const
	{
		return crntToken;
	}

	/// Get the name of the input stream
	const char* getScriptName() const
	{
		return scriptName;
	}

	/// Get the current line the Scanner is processing
	int getLineNumber() const
	{
		return lineNmbr;
	}

protected:
	/// Every char in the Ascii table is binded with one characteristic
	/// code type. This helps the scanning
	enum AsciiFlag
	{
		AC_ERROR = 0,
		AC_EOF = 1,
		AC_LETTER = 2,
		AC_DIGIT = 4,
		AC_SPECIAL = 8,
		AC_WHITESPACE = 16,
		AC_QUOTE = 32,
		AC_DOUBLEQUOTE = 64,
		AC_ACCEPTABLE_IN_COMMENTS = 128 ///< Only accepted in comments
	};

	/// Reserved words like "int" "const" etc. Currently the reserved words
	/// list is being populated with dummy data
	struct ResWord
	{
		const char* string;
		TokenCode   code;
	};

	static char eofChar; ///< Special end of file character

	/// The array contains one AsciiFlag for every symbol of the ASCII table
	static AsciiFlag asciiLookupTable[];

	/// @name Reserved words
	/// Groups of ResWord grouped by the length of the ResWord::string
	/// @{
	static ResWord rw2[], rw3[], rw4[], rw5[], rw6[], rw7[];
	/// @}

	/// The array contains all the groups of ResWord
	static ResWord* rwTable[];

	Token crntToken; ///< The current token
	/// In contains the current line's text
	char line[MAX_SCRIPT_LINE_LEN];
	char* pchar; ///< Points somewhere to @ref line
	int lineNmbr; ///< The number of the current line

	/// Treat newlines as whitespace. If false means that the Scanner
	/// returns (among others) newline tokens
	bool newlinesAsWhitespace;

	/// Commented lines number
	/// Used to keep track of the newlines in multiline comments so we can
	/// then return the correct number of newlines
	/// in case of newlinesAsWhitespace is false
	int commentedLines;

	/// @name Input
	/// @{

	/// The file stream. Used if the @ref Scanner is initiated using
	/// @ref loadFile
	std::ifstream inFstream;
	/// Points to either @ref inFstream or an external std::istream
	std::istream* inStream;
	/// The name of the input stream. Mainly used for error messaging
	char scriptName[512];
	/// @}

	/// @name Checkers
	/// @{
	void checkWord();
	void checkComment();
	void checkNumber();
	void checkString();
	void checkChar();
	void checkSpecial();
	/// @}

	/// It reads a new line from the iostream and it points @ref pchar to
	/// the beginning of that line
	void getLine();

	/// Get the next char from the @ref line. If @ref line empty then get
	/// new line. It returns '\\0' if we are in the
	/// end of the line
	char getNextChar();

	/// Put the char that @ref getNextChar got back to the current line
	char putBackChar();

	/// Initializes the asciiLookupTable. It runs only once in the
	/// construction of the first Scanner @see Scanner()
	static void initAsciiMap();

	/// A function to save us from typing
	static AsciiFlag& lookupAscii(char c)
	{
		return asciiLookupTable[static_cast<int>(c)];
	}

	/// Common initialization code
	void init(bool newlinesAsWhitespace_);

	/// Unloads the file
	void unload();
};



}} // end namespaces


#endif
