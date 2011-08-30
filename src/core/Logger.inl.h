
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


inline Logger::Info Logger::setInfo(const char* file, int line,
	const char* func, MessageType type)
{
	Info sender = {file, line, func, type};
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


template<typename F, typename T>
void Logger::connect(F f, T t)
{
	sig.connect(boost::bind(f, t, _1, _2, _3, _4, _5));
}
