#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "engine_class.h"

char nc_t::unamed[] = "unamed";

// nc_t
nc_t::nc_t()
{
	name = unamed; // the default value of name
}

// ~nc_t
nc_t::~nc_t()
{
	if( name != unamed ) free(name);
}

// SetName
void nc_t::SetName( const char* name_ )
{
	if( name != unamed ) free(name);

	name = (char*)malloc( (strlen(name_) + 1) * sizeof(char) );
	strcpy( name, name_ );

	if( strlen(name) > 30 )
		WARNING( "Big name for: \"" << name << '\"' );
}
