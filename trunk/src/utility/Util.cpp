#include "Util.h"
#include <math.h>


namespace Util {

//=====================================================================================================================================
// randRange                                                                                                                          =
//=====================================================================================================================================
int randRange( int min, int max )
{
	return (rand() % (max-min+1)) + min ;
}


float randRange( float min, float max )
{
	double d = max - min; // difference
	if( d==0.0 ) return min;
	// diferrense = mant * e^exp
	int exp;
	double mant = frexp( d, &exp );

	int precision = 1000; // more accurate

	mant *= precision;
	double new_mant = rand() % (int)mant;
	new_mant /= precision;

	return min + (float)ldexp( new_mant, exp ); // return min + (new_mant * e^exp)
}


//=====================================================================================================================================
// readFile                                                                                                                           =
//=====================================================================================================================================
string readFile( const char* filename )
{
	ifstream file( filename );
	if ( !file.is_open() )
	{
		ERROR( "Cannot open file \"" << filename << "\"" );
		return string();
	}

	return string( (istreambuf_iterator<char>(file)), istreambuf_iterator<char>() );
}


//=====================================================================================================================================
// getFileLines                                                                                                                       =
//=====================================================================================================================================
Vec<string> getFileLines( const char* filename )
{
	Vec<string> lines;
	ifstream ifs( filename );
	if( !ifs.is_open() )
	{
		ERROR( "Cannot open file \"" << filename << "\"" );
		return lines;
	}

  string temp;
  while( getline( ifs, temp ) )
  {
  	lines.push_back( temp );
  }
  return lines;
}


//=====================================================================================================================================
// cutPath                                                                                                                            =
//=====================================================================================================================================
/// Used only to cut the path from __FILE__ and return the actual file name and some other cases
char* cutPath( const char* path )
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if( (str-path)<=-1 || *str=='/' || *str=='\\'  )  break;
		str--;
	}
	return str+1;
}


//=====================================================================================================================================
// getRsrcPath                                                                                                                            =
//=====================================================================================================================================
string getPath( const char* path )
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if( (str-path)<=-1 || *str=='/' || *str=='\\'  )  break;
		-- str;
	}
	++str;
	int n = str - path;
	DEBUG_ERR( n<0 || n>100 ); // check the func. probably something wrong
	string ret_str;
	ret_str.assign( path, n );
	return ret_str;
}


//=====================================================================================================================================
// getFunctionFromPrettyFunction                                                                                                      =
//=====================================================================================================================================
/// The function gets __PRETTY_FUNCTION__ and strips it to get only the function name with its namespace
string getFunctionFromPrettyFunction( const char* pretty_function )
{
	string ret( pretty_function );

	size_t index = ret.find( "(" );

	if ( index != string::npos )
		ret.erase( index );

	index = ret.rfind( " " );
	if ( index != string::npos )
		ret.erase( 0, index + 1 );

	return ret;
}


//=====================================================================================================================================
// getFileExtension                                                                                                                   =
//=====================================================================================================================================
char* getFileExtension( const char* path )
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if( (str-path)<=-1 || *str=='.' )  break;
		str--;
	}

	if( str == (char*)path + strlen(path) - 1 ) ERROR( "Please puth something after the '.' in path \"" << path << "\". Idiot" );
	if( str+1 == path ) ERROR( "Path \"" << path << "\" doesnt contain a '.'. What the fuck?" );

	return str+1;
}


//=====================================================================================================================================
// intToStr                                                                                                                           =
//=====================================================================================================================================
string intToStr( int i )
{
	char str [256];
	char* pstr = str + ( (sizeof(str)/sizeof(char)) - 1 );
	bool negative = false;

	*pstr = '\0';

	if( i < 0 )
	{
		i = -i;
		negative = true;
	}

	do
	{
		--pstr;
		int remain = i % 10;
		i = i / 10;
		*pstr = '0' + remain;
	} while( i != 0 );

	if( negative )
	{
		--pstr;
		*pstr = '-';
	}

	return string(pstr);
}


//=====================================================================================================================================
// floatToStr                                                                                                                         =
//=====================================================================================================================================
string floatToStr( float f )
{
	char tmp [128];

	sprintf( tmp, "%f", f );

	string s(tmp);
	return s;
}


}
