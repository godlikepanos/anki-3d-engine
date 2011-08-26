
inline Logger& Logger::operator<<(const bool& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const short& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const unsigned short& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const int& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const unsigned int& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const long& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const unsigned long& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const float& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const double& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger& Logger::operator<<(const long double& val)
{
	return appendUsingLexicalCast(val);
}


inline Logger::LoggerSender Logger::setSender(const char* file, int line,
	const char* func)
{
	LoggerSender sender = {file, line, func};
	return sender;
}


template<typename Type>
Logger& Logger::appendUsingLexicalCast(const Type& val)
{
	std::string out;
	try
	{
		out = boost::lexical_cast<std::string>(val);
	}
	catch(...)
	{
		out = "*error*";
	}
	append(out.c_str(), out.length());
	return *this;
}
