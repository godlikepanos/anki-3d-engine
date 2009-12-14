#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <string>
#include <math.h>
#include "common.h"

/*
=======================================================================================================================================
RandRange                                                                                                                             =
=======================================================================================================================================
*/
int RandRange( int min, int max )
{
	return (rand() % (max-min+1)) + min ;
}


float RandRange( float min, float max )
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


/*
=======================================================================================================================================
ReadFile                                                                                                                              =
=======================================================================================================================================
*/
string ReadFile( const char* filename )
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
// GetFileLines                                                                                                                       =
//=====================================================================================================================================
vec_t<string> GetFileLines( const char* filename )
{
	vec_t<string> lines;
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


/*
=======================================================================================================================================
CutPath                                                                                                                               =
=======================================================================================================================================
*/
/// Used only to cut the path from __FILE__ and return the actual file name and some other cases
char* CutPath( const char* path )
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if( (str-path)<=-1 || *str=='/' || *str=='\\'  )  break;
		str--;
	}
	return str+1;
}


/*
=======================================================================================================================================
GetPath                                                                                                                               =
=======================================================================================================================================
*/
string GetPath( const char* path )
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


/*
=======================================================================================================================================
GetFunctionFromPrettyFunction                                                                                                         =
=======================================================================================================================================
*/
/// The function gets __PRETTY_FUNCTION__ and strips it to get only the function name with its namespace
string GetFunctionFromPrettyFunction( const char* pretty_function )
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


/*
=======================================================================================================================================
GetFileExtension                                                                                                                      =
=======================================================================================================================================
*/
char* GetFileExtension( const char* path )
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


/*
=======================================================================================================================================
Benchmark stuff                                                                                                                       =
=======================================================================================================================================
*/
clock_t bench_ms = 0;

void StartBench()
{
	if( bench_ms )
		ERROR( "Bench is allready running" );

	bench_ms = clock();
}

clock_t StopBench()
{
	clock_t t = clock() - bench_ms;
	bench_ms = 0;
	return t/1000;
}


/*
=======================================================================================================================================
IntToStr                                                                                                                              =
=======================================================================================================================================
*/
string IntToStr( int i )
{
	string s = "";
	if( i == 0 )
	{
		s = "0";
		return s;
	}
	if( i < 0 )
	{
		s += '-';
		i = -i;
	}
	int count = log10(i);
	while( count >= 0 )
	{
		s += ('0' + i/pow(10.0, count));
		i -= static_cast<int>(i/pow(10.0,count)) * static_cast<int>(pow(10.0,count));
		count--;
	}
	return s;
}


/*
=======================================================================================================================================
FloatToStr                                                                                                                            =
=======================================================================================================================================
*/
string FloatToStr( float f )
{
	char tmp [128];

	sprintf( tmp, "%f", f );

	string s(tmp);
	return s;
}



/*
=======================================================================================================================================
HudPrintMemInfo                                                                                                                       =
=======================================================================================================================================
*/
void HudPrintMemInfo()
{
	/*hud::Printf( "==== Mem ===\n" );
	hud::Printf( "Buffer:\nfree:%ldb total:%ldb\n", mem::free_size, mem::buffer_size );
	hud::Printf( "Calls count:\n" );
	hud::Printf( "m:%ld c:%ld r:%ld\nf:%ld n:%ld d:%ld\n", mem::malloc_called_num, mem::calloc_called_num, mem::realloc_called_num, mem::free_called_num, mem::new_called_num, mem::delete_called_num );*/
}

