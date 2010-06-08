#include "Common.h"
#include "App.h"
#include "Util.h"


// for the colors and formating see http://www.dreamincode.net/forums/showtopic75171.htm
static const char* terminalColors [ MT_NUM + 1 ] = {
	"\033[1;31;6m", // error
	"\033[1;33;6m", // warning
	"\033[1;31;6m", // fatal
	"\033[1;31;6m", // dbg error
	"\033[1;32;6m", // info
	"\033[0;;m" // default color
};


//======================================================================================================================
// msgPrefix                                                                                                           =
//======================================================================================================================
ostream& msgPrefix( MsgType msgType, const char* file, int line, const char* func )
{
	if( app == NULL )
		::exit( 1 );

	// select c stream
	ostream* cs;

	switch( msgType )
	{
		case MT_ERROR:
		case MT_WARNING:
		case MT_FATAL:
		case MT_DEBUG_ERR:
			cs = &cerr;
			break;

		case MT_INFO:
			cs = &cout;
			break;

		default:
			break;
	}


	// print terminal color
	if( app->isTerminalColoringEnabled() )
	{
		(*cs) << terminalColors[ msgType ];
	}


	// print message info
	switch( msgType )
	{
		case MT_ERROR:
			(*cs) << "Error";
			break;

		case MT_WARNING:
			(*cs) << "Warning";
			break;

		case MT_FATAL:
			(*cs) << "Fatal";
			break;

		case MT_DEBUG_ERR:
			(*cs) << "Debug Err";
			break;

		case MT_INFO:
			(*cs) << "Info";
			break;

		default:
			break;
	}

	// print caller info
	(*cs) << " (" << Util::cutPath( file ) << ":" << line << " " << Util::getFunctionFromPrettyFunction( func ) << "): ";

	return (*cs);
}


//======================================================================================================================
// msgSuffix                                                                                                           =
//======================================================================================================================
ostream& msgSuffix( ostream& cs )
{
	if( app == NULL )
		::exit( 1 );

	if( app->isTerminalColoringEnabled() )
		cs << terminalColors[ MT_NUM ];

	cs << endl;
	return cs;
}

