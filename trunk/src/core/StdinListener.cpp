#include "anki/core/StdinListener.h"
#include <array>
#include <unistd.h>

namespace anki {

//==============================================================================
void StdinListener::workingFunc()
{
	std::array<char, 512> buff;

	while(1)
	{
		int m = read(0, &buff[0], sizeof(buff));
		buff[m] = '\0';
		//cout << "read: " << buff << endl;
		{
			std::lock_guard<std::mutex> lock(mtx);
			q.push(&buff[0]);
			//cout << "size:" << q.size() << endl;
		}
	}
}

//==============================================================================
std::string StdinListener::getLine()
{
	std::string ret;
	std::lock_guard<std::mutex> lock(mtx);
	//cout << "_size:" << q.size() << endl;
	if(!q.empty())
	{
		ret = q.front();
		q.pop();
	}
	return ret;
}

//==============================================================================
void StdinListener::start()
{
	thrd = std::thread(&StdinListener::workingFunc, this);
}

} // end namespace
