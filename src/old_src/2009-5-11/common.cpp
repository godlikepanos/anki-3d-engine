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
char* ReadFile( const char* filename )
{
	fstream file( filename, ios::in | ios::ate );
  if ( !file.is_open() )
  {
		ERROR( "Canot open file " << filename );
		return NULL;
  }
  char* memblock;
  int size;
	size = file.tellg();
	memblock = new char [size+1];
	file.seekg( 0, ios::beg );
	file.read( memblock, size );
	memblock[size] = '\0';
	file.close();
	return memblock;
}


/*
=======================================================================================================================================
CutPath                                                                                                                               =
used only to cut the path from __FILE__ and return the actual file name                                                               =
=======================================================================================================================================
*/
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

