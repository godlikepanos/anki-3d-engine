#include "skel_anim.h"
#include "scanner.h"
#include "parser.h"


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool skel_anim_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;


	// keyframes
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT  )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	keyframes.resize( token->value.int_ );

	if( !ParseArrOfNumbers( scanner, false, false, keyframes.size(), &keyframes[0] ) ) return false;

	// bones num
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT  )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	bones.resize( token->value.int_ );

	// poses
	for( uint i=0; i<bones.size(); i++ )
	{
		// has anim?
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT  )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}

		// it has
		if( token->value.int_ == 1 )
		{
			bones[i].keyframes.resize( keyframes.size() );

			for( uint j=0; j<keyframes.size(); ++j )
			{
				// parse the quat
				float tmp[4];
				if( !ParseArrOfNumbers( scanner, false, true, 4, &tmp[0] ) ) return false;
				bones[i].keyframes[j].rotation = quat_t( tmp[1], tmp[2], tmp[3], tmp[0] );

				// parse the vec3
				if( !ParseArrOfNumbers( scanner, false, true, 3, &bones[i].keyframes[j].translation[0] ) ) return false;
			}
		}
	} // end for all bones


	frames_num = keyframes[ keyframes.size()-1 ] + 1;

	return true;
}
