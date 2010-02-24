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
HudprintMemInfo                                                                                                                       =
=======================================================================================================================================
*/
void HudprintMemInfo()
{
	/*hud::printf( "==== Mem ===\n" );
	hud::printf( "Buffer:\nfree:%ldb total:%ldb\n", mem::free_size, mem::buffer_size );
	hud::printf( "Calls count:\n" );
	hud::printf( "m:%ld c:%ld r:%ld\nf:%ld n:%ld d:%ld\n", mem::malloc_called_num, mem::calloc_called_num, mem::realloc_called_num, mem::free_called_num, mem::new_called_num, mem::delete_called_num );*/
}

