#include "Skeleton.h"
#include "Scanner.h"
#include "Parser.h"


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
	if( token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT )
	{
		PARSE_ERR_EXPECTED( "integer" );
		return false;
	}
	bones.resize( token->getValue().getInt(), Bone() );

	for( uint i=0; i<bones.size(); i++ )
	{
		Bone& bone = bones[i];
		bone.id = i;

		// NAME
		token = &scanner.getNextToken();
		if( token->getCode() != Scanner::TC_STRING )
		{
			PARSE_ERR_EXPECTED( "string" );
			return false;
		}
		bone.setName( token->getValue().getString() );

		// head
		if( !Parser::parseArrOfNumbers<float>( scanner, false, true, 3, &bone.head[0] ) ) return false;

		// tail
		if( !Parser::parseArrOfNumbers<float>( scanner, false, true, 3, &bone.tail[0] ) ) return false;

		// matrix
		Mat4 m4;
		if( !Parser::parseArrOfNumbers<float>( scanner, false, true, 16, &m4[0] ) ) return false;

		// matrix for real
		bone.rotSkelSpace = m4.getRotationPart();
		bone.tslSkelSpace = m4.getTranslationPart();
		Mat4 MAi( m4.getInverse() );
		bone.rotSkelSpaceInv = MAi.getRotationPart();
		bone.tslSkelSpaceInv = MAi.getTranslationPart();

		// parent
		token = &scanner.getNextToken();
		if( (token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT) &&
		    (token->getCode() != Scanner::TC_IDENTIFIER || strcmp( token->getValue().getString(), "NULL" )!=0) )
		{
			PARSE_ERR_EXPECTED( "integer or NULL" );
			return false;
		}

		if( token->getCode() == Scanner::TC_IDENTIFIER )
			bone.parent = NULL;
		else
			bone.parent = &bones[ token->getValue().getInt() ];

		// childs
		token = &scanner.getNextToken();
		if( token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT )
		{
			PARSE_ERR_EXPECTED( "integer" );
			return false;
		}
		if( token->getValue().getInt() > Bone::MAX_CHILDS_PER_BONE )
		{
			ERROR( "Childs for bone \"" << bone.getName() << "\" exceed the max" );
			return false;
		}
		bone.childsNum = token->getValue().getInt();
		for( int j=0; j<bone.childsNum; j++ )
		{
			token = &scanner.getNextToken();
			if( token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT )
			{
				PARSE_ERR_EXPECTED( "integer" );
				return false;
			}
			bone.childs[j] = &bones[ token->getValue().getInt() ];
		}
	} // for all bones

	return true;
}
