// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Process.h>
#include <anki/util/Functions.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

namespace anki
{

Process::~Process()
{
	ProcessStatus s;
	const Error err = getStatus(s);
	if(err)
	{
		ANKI_UTIL_LOGE("Failed to get the status. Expect cleanup errors");
		return;
	}

	if(s == ProcessStatus::RUNNING)
	{
		ANKI_UTIL_LOGE("Destroying while child process is running");
		return;
	}

	destroyPipes();
}

void Process::destroyPipes()
{
	for(U32 i = 0; i < 2; ++i)
	{
		close(m_stdoutPipe[i]);
		close(m_stderrPipe[i]);
		m_stdoutPipe[i] = m_stderrPipe[i] = -1;
	}
}

Error Process::createPipes()
{
	destroyPipes();

	if(pipe(m_stdoutPipe.getBegin()) || pipe(m_stderrPipe.getBegin()))
	{
		ANKI_UTIL_LOGE("pipe() failed: %s", strerror(errno));
		return Error::FUNCTION_FAILED;
	}

	fcntl(m_stdoutPipe[0], F_SETFL, O_NONBLOCK);
	fcntl(m_stderrPipe[0], F_SETFL, O_NONBLOCK);

	return Error::NONE;
}

Error Process::start(CString executable, ConstWeakArray<CString> arguments, ConstWeakArray<CString> environment)
{
	ANKI_CHECK(refresh(0));
	if(m_status == ProcessStatus::RUNNING)
	{
		ANKI_UTIL_LOGE("Process already running");
		return Error::USER_DATA;
	}

	// Create the pipes
	ANKI_CHECK(createPipes());

	// Fork
	m_pid = fork();
	if(m_pid < 0)
	{
		ANKI_UTIL_LOGE("fork() failed: %s", strerror(errno));
	}

	if(m_pid == 0)
	{
		// Child

		dup2(m_stdoutPipe[1], 1);
		close(m_stdoutPipe[0]);
		m_stdoutPipe[0] = -1;

		dup2(m_stderrPipe[1], 2);
		close(m_stderrPipe[0]);
		m_stderrPipe[0] = -1;

		// Set the args
		char** cargs = static_cast<char**>(malloc((arguments.getSize() + 2) * sizeof(char*)));
		cargs[0] = static_cast<char*>(malloc(executable.getLength() + 1));
		strcpy(cargs[0], executable.cstr());
		U32 i = 0;
		for(; i < arguments.getSize(); ++i)
		{
			cargs[i + 1] = static_cast<char*>(malloc(arguments[i].getLength() + 1));
			strcpy(cargs[i + 1], arguments[i].cstr());
		}
		cargs[i + 1] = nullptr;

		// Set the env
		char** newenv = static_cast<char**>(malloc((environment.getSize() + 1) * sizeof(char*)));
		i = 0;
		for(; i < environment.getSize(); ++i)
		{
			newenv[i] = static_cast<char*>(malloc(environment[i].getLength() + 1));
			strcpy(newenv[i], environment[i].cstr());
		}
		newenv[i] = nullptr;

		// Execute file
		const int execerror = execvpe(executable.cstr(), cargs, newenv);
		if(execerror)
		{
			printf("execvpe() failed: %s", strerror(errno));
			exit(1);
		}
	}
	else
	{
		close(m_stdoutPipe[1]);
		m_stdoutPipe[1] = -1;
		close(m_stderrPipe[1]);
		m_stderrPipe[1] = -1;
	}

	return Error::NONE;
}

Error Process::kill(ProcessKillSignal k)
{
	if(m_pid < 1)
	{
		return Error::NONE;
	}

	int sig;
	switch(k)
	{
	case ProcessKillSignal::NORMAL:
		sig = SIGTERM;
		break;
	case ProcessKillSignal::FORCE:
		sig = SIGKILL;
		break;
	default:
		ANKI_ASSERT(0);
		sig = 0;
	};

	const pid_t p = ::kill(m_pid, sig);
	if(p != 0)
	{
		ANKI_UTIL_LOGE("kill() failed: %s", strerror(errno));
		return Error::FUNCTION_FAILED;
	}

	return Error::NONE;
}

Error Process::readFromFd(int fd, StringAuto& text) const
{
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	timeval timeout;
	timeout.tv_sec = 0; // Seconds
	timeout.tv_usec = 10; // Microseconds

	// Limit the iterations in case the process writes to FD constantly
	U32 maxIterations = 16;
	while(maxIterations--)
	{
		// Check if there are data
		const int sel = select(1 + fd, &readfds, nullptr, nullptr, &timeout);
		if(sel == 0)
		{
			// Timeout expired
			break;
		}
		else if(sel == -1)
		{
			ANKI_UTIL_LOGE("select() failed: %s", strerror(errno));
			return Error::FUNCTION_FAILED;
		}

		// Read the data
		Array<char, 1024> buff;
		const ssize_t bytesCount = read(fd, buff.getBegin(), buff.getSize() - 1);

		if(bytesCount < 0 && errno != EAGAIN)
		{
			// NOTE errno == EINTR if a non-fatal signal arrived
			ANKI_UTIL_LOGE("read() failed: %s", strerror(errno));
			return Error::FUNCTION_FAILED;
		}
		else if(bytesCount == 0)
		{
			break;
		}
		else
		{
			buff[min<U32>(U32(bytesCount), buff.getSize() - 1)] = '\0';
			text.append(&buff[0]);
		}
	}

	return Error::NONE;
}

Error Process::readFromStdout(StringAuto& text)
{
	return readFromFd(m_stdoutPipe[0], text);
}

Error Process::readFromStderr(StringAuto& text)
{
	return readFromFd(m_stderrPipe[0], text);
}

Error Process::getStatus(ProcessStatus& status)
{
	ANKI_CHECK(refresh(WNOHANG));
	status = m_status;
	return Error::NONE;
}

Error Process::refresh(int waitpidOptions)
{
	Error err = Error::NONE;
	m_status = ProcessStatus::NOT_RUNNING;
	m_exitCode = DEFAULT_EXIT_CODE;

	if(m_pid == -1)
	{
		m_status = ProcessStatus::NOT_RUNNING;
		m_exitCode = DEFAULT_EXIT_CODE;
	}
	else
	{
		int status;
		const pid_t p = waitpid(m_pid, &status, waitpidOptions);

		if(p == -1)
		{
			m_status = ProcessStatus::NOT_RUNNING;
			m_exitCode = DEFAULT_EXIT_CODE;
			m_pid = -1;
			ANKI_UTIL_LOGE("waitpid() failed: %s", strerror(errno));
			err = Error::FUNCTION_FAILED;
		}
		else if(p == 0)
		{
			m_status = ProcessStatus::RUNNING;
			m_exitCode = DEFAULT_EXIT_CODE;
		}
		else if(WIFEXITED(status))
		{
			m_status = ProcessStatus::NORMAL_EXIT;
			m_exitCode = WEXITSTATUS(status);
			m_pid = -1;
		}
		else if(WIFSIGNALED(status))
		{
			m_status = ProcessStatus::CRASH_EXIT;
			m_exitCode = WTERMSIG(status);
			m_pid = -1;
		}
		else if(WIFSTOPPED(status))
		{
			// NOTE child may resume later (and become a zombie)
			m_status = ProcessStatus::CRASH_EXIT;
			m_exitCode = WSTOPSIG(status);
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}

	return err;
}

Error Process::wait(Second timeout, ProcessStatus* status, I32* exitCode)
{
	ANKI_CHECK(refresh(0));

	if(status)
	{
		*status = m_status;
	}

	if(exitCode)
	{
		*exitCode = m_exitCode;
	}

	return Error::NONE;
}

} // end namespace anki
