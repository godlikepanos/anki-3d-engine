#include "Skeleton.h"
#include "Scanner.h"
#include "parser.h"


//=====================================================================================================================================
// load                                                                                                                               =
//=====================================================================================================================================
bool Skeleton::load( const char* filename )
{
	Scanner scanner;
	if( !scanner.loadFile( filename ) ) return false;

	const Scanner::Token* token;


	//** BONES NUM **
	token = &scanner.getNextToken();
	if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	bones.resize( token->value.int_, Bone() );

	for( uint i=0; i<bones.size(); i++ )
	{
		Bone& bone = bones[i];
		bone.id = i;

		// NAME
		token = &scanner.getNextToken();
		if( token->code != Scanner::TC_STRING )
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
		Mat4 m4;
		if( !ParseArrOfNumbers<float>( scanner, false, true, 16, &m4[0] ) ) return false;

		// matrix for real
		bone.rotSkelSpace = m4.getRotationPart();
		bone.tslSkelSpace = m4.getTranslationPart();
		Mat4 MAi( m4.getInverse() );
		bone.rotSkelSpaceInv = MAi.getRotationPart();
		bone.tslSkelSpaceInv = MAi.getTranslationPart();

		// parent
		token = &scanner.getNextToken();
		if( (token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT) &&
		    (token->code != Scanner::TC_IDENTIFIER || strcmp( token->value.string, "NULL" )!=0) )
		{
			PARSE_ERR_EXPECTED( "integer or NULL" );
			return false;
		}

		if( token->code == Scanner::TC_IDENTIFIER )
			bone.parent = NULL;
		else
			bone.parent = &bones[ token->value.int_ ];

		// childs
		token = &scanner.getNextToken();
		if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}
		if( token->value.int_ > Bone::MAX_CHILDS_PER_BONE )
		{
			ERROR( "Childs for bone \"" << bone.GetName() << "\" exceed the max" );
			return false;
		}
		bone.childsNum = token->value.int_;
		for( int j=0; j<bone.childsNum; j++ )
		{
			token = &scanner.getNextToken();
			if( token->code != Scanner::TC_NUMBER || token->type != Scanner::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			bone.childs[j] = &bones[ token->value.int_ ];
		}
	} // for all bones

	return true;
}
