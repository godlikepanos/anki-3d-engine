#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <string>
#include <math.h>
#include "common.h"


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

