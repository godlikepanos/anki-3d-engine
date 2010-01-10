#include "skeleton.h"
#include "scanner.h"
#include "parser.h"


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool skeleton_t::Load( const char* filename )
{
	scanner_t scanner;
	if( !scanner.LoadFile( filename ) ) return false;

	const scanner_t::token_t* token;


	//** BONES NUM **
	token = &scanner.GetNextToken();
	if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	bones.resize( token->value.int_, bone_t() );

	for( uint i=0; i<bones.size(); i++ )
	{
		bone_t& bone = bones[i];
		bone.id = i;

		// NAME
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_STRING )
		{
			PARSE_ERR_EXPECTED( "string" );
			return false;
		}
		bone.SetName( token->value.string );

		// head
		if( !ParseArrOfNumbers<float>( scanner, false, true, 3, &bone.head[0] ) ) return false;

		// tail
		if( !ParseArrOfNumbers<float>( scanner, false, true, 3, &bone.tail[0] ) ) return false;

		// matrix
		mat4_t m4;
		if( !ParseArrOfNumbers<float>( scanner, false, true, 16, &m4[0] ) ) return false;

		// matrix for real
		bone.rot_skel_space = m4.GetRotationPart();
		bone.tsl_skel_space = m4.GetTranslationPart();
		mat4_t MAi( m4.GetInverse() );
		bone.rot_skel_space_inv = MAi.GetRotationPart();
		bone.tsl_skel_space_inv = MAi.GetTranslationPart();

		// parent
		token = &scanner.GetNextToken();
		if( (token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT) &&
		    (token->code != scanner_t::TC_IDENTIFIER || strcmp( token->value.string, "NULL" )!=0) )
		{
			PARSE_ERR_EXPECTED( "integer or NULL" );
			return false;
		}

		if( token->code == scanner_t::TC_IDENTIFIER )
			bone.parent = NULL;
		else
			bone.parent = &bones[ token->value.int_ ];

		// childs
		token = &scanner.GetNextToken();
		if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}
		if( token->value.int_ > bone_t::MAX_CHILDS_PER_BONE )
		{
			ERROR( "Childs for bone \"" << bone.GetName() << "\" exceed the max" );
			return false;
		}
		bone.childs_num = token->value.int_;
		for( int j=0; j<bone.childs_num; j++ )
		{
			token = &scanner.GetNextToken();
			if( token->code != scanner_t::TC_NUMBER || token->type != scanner_t::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			bone.childs[j] = &bones[ token->value.int_ ];
		}
	} // for all bones

	return true;
}
