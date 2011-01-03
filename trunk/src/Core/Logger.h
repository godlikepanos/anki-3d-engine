#ifndef LOGGER_H
#define LOGGER_H

#include <boost/array.hpp>


/// The logger singleton class
class Logger
{
	public:
		static Logger& getInstance();

		/*Logger& operator<<(const bool& val);
		Logger& operator<<(const short& val);
		Logger& operator<<(const unsigned short& val);
		Logger& operator<<(const int& val);
		Logger& operator<<(const unsigned int& val);
		Logger& operator<<(const long& val);
		Logger& operator<<(const unsigned long& val);
		Logger& operator<<(const float& val);
		Logger& operator<<(const double& val);
		Logger& operator<<(const long double& val);
		Logger& operator<<(const void* val);*/
		Logger& operator<<(const char* val);

		Logger& operator<<(Logger& (*funcPtr)(Logger&));


	private:
		static Logger* instance;
		static const int STREAM_SIZE = 512;
		boost::array<char, STREAM_SIZE> stream;

		Logger() {}
		Logger(const Logger&) {}
		void operator=(const Logger&) {}
};


inline Logger& Logger::getInstance()
{
	if(instance == NULL)
	{
		instance = new Logger();
	}
	return *instance;
}


/// @todo
inline Logger& endl(Logger& in) {}


#endif
