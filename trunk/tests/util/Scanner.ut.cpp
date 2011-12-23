#include "anki/util/Scanner.h"
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <memory>


TEST(Scanner, Numbers)
{
	std::stringstream ss;	
	ss << "12345678901234 1.12 0.00000000000001 0.01e1 1e- 10.123e-7 1ab";

	std::auto_ptr<scanner::Scanner> scanner_;
	const scanner::Token* token;
	
	EXPECT_NO_THROW(scanner_.reset(new scanner::Scanner(ss, "numbers")));
	scanner::Scanner& scanner = *scanner_;

	// 12345678901234
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_NUMBER);	
	EXPECT_EQ(token->getDataType(), scanner::DT_INT);	
	EXPECT_EQ(token->getValue().getInt(), 12345678901234);
	
	// 1.12
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_NUMBER);	
	EXPECT_EQ(token->getDataType(), scanner::DT_FLOAT);	
	EXPECT_FLOAT_EQ(token->getValue().getFloat(), 1.12);
	
	// 0.00000000000001
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_NUMBER);	
	EXPECT_EQ(token->getDataType(), scanner::DT_FLOAT);	
	EXPECT_FLOAT_EQ(token->getValue().getFloat(), 0.00000000000001);
	
	// 0.01e1
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_NUMBER);	
	EXPECT_EQ(token->getDataType(), scanner::DT_FLOAT);	
	EXPECT_FLOAT_EQ(token->getValue().getFloat(), 0.01e1);
	
	// 1e-
	EXPECT_ANY_THROW(token = &scanner.getNextToken());
	
	// 10.123e-7
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_NUMBER);	
	EXPECT_EQ(token->getDataType(), scanner::DT_FLOAT);	
	EXPECT_FLOAT_EQ(token->getValue().getFloat(), 10.123e-7);
	
	// 1ab
	EXPECT_ANY_THROW(token = &scanner.getNextToken());
	
	// EOF
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_END);
}


TEST(Scanner, Identifiers)
{
	std::stringstream ss;	
	ss << "1 la0_la ha\n_ha";

	std::auto_ptr<scanner::Scanner> scanner_;
	const scanner::Token* token;
	
	EXPECT_NO_THROW(scanner_.reset(new scanner::Scanner(ss, "identifiers")));
	scanner::Scanner& scanner = *scanner_;
	
	// 1
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_NE(token->getCode(), scanner::TC_IDENTIFIER);
	
	// la_la
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_IDENTIFIER);	
	EXPECT_EQ(token->getValue().getString(), std::string("la0_la"));
	
	// ha
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_IDENTIFIER);	
	EXPECT_EQ(token->getValue().getString(), std::string("ha"));
	
	// _ha
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_IDENTIFIER);	
	EXPECT_EQ(token->getValue().getString(), std::string("_ha"));
	
	// EOF
	EXPECT_NO_THROW(token = &scanner.getNextToken());
	EXPECT_EQ(token->getCode(), scanner::TC_END);
}





