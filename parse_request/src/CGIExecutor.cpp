#include "../inc/CGIExecutor.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <sys/select.h>
#include <iostream>
#include <sstream>

char** CGIExecutor::mapToEnvArray(const std::map<std::string, std::string>& envVars)
{
	// Allocate array for environment variables 
	char** envArray = new char*[envVars.size() + 1];
	
	size_t index = 0;
	for (std::map<std::string, std::string>::const_iterator it = envVars.begin();
		 it != envVars.end(); ++it)
	{

		std::string envStr = it->first + "=" + it->second;
		envArray[index] = new char[envStr.length() + 1];
		std::strcpy(envArray[index], envStr.c_str());
		index++;
	}
	envArray[envVars.size()] = NULL;
	
	return envArray;
}

// Free environment array
void CGIExecutor::freeEnvArray(char** envArray, size_t count)
{
	if (!envArray)
		return;
	
	for (size_t i = 0; i < count; ++i)
	{
		if (envArray[i])
			delete[] envArray[i];
	}
	delete[] envArray;
}

// Read from file descriptor with timeout
std::string CGIExecutor::readWithTimeout(int fd, size_t timeout)
{
	std::string result;
	char buffer[4096];
	
	while (true)
	{
		fd_set readFds;
		FD_ZERO(&readFds);
		FD_SET(fd, &readFds);
		
		struct timeval tv;
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		
		int selectResult = select(fd + 1, &readFds, NULL, NULL, timeout > 0 ? &tv : NULL);
		
		if (selectResult < 0)
		{
			// Error in select
			break;
		}
		else if (selectResult == 0)
		{
			// Timeout
			break;
		}
		else if (FD_ISSET(fd, &readFds))
		{
			ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
			if (bytesRead <= 0)
				break; // EOF or error
			
			buffer[bytesRead] = '\0';
			result += buffer;
		}
	}
	
	return result;
}

CGIExecutor::CGIResult CGIExecutor::executeCGI(
	const std::string& scriptPath,
	const std::string& interpreterPath,
	const std::map<std::string, std::string>& envVars,
	const std::string& requestBody,
	size_t timeout
)
{
	CGIResult result;
	
	int stdinPipe[2];
	int stdoutPipe[2];
	
	if (pipe(stdinPipe) == -1)
	{
		result.statusCode = 500;
		result.error = "Failed to create stdin pipe";
		return result;
	}
	
	if (pipe(stdoutPipe) == -1)
	{
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		result.statusCode = 500;
		result.error = "Failed to create stdout pipe";
		return result;
	}
	
	pid_t pid = fork();
	
	if (pid == -1)
	{
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		close(stdoutPipe[0]);
		close(stdoutPipe[1]);
		result.statusCode = 500;
		result.error = "Failed to fork child process";
		return result;
	}
	
	if (pid == 0)
	{
		
		close(stdinPipe[1]);
		close(stdoutPipe[0]);
		
		if (dup2(stdinPipe[0], STDIN_FILENO) == -1)
		{
			std::cerr << "dup2 stdin failed\n";
			exit(500);
		}
		close(stdinPipe[0]);
		
		if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
		{
			std::cerr << "dup2 stdout failed\n";
			exit(500);
		}
		close(stdoutPipe[1]);
		
		char** envArray = mapToEnvArray(envVars);
		
		const char* argv[3];
		if (!interpreterPath.empty()) //(e.g., /usr/bin/php)
		{
			argv[0] = interpreterPath.c_str();
			argv[1] = scriptPath.c_str();
			argv[2] = NULL;
		}
		else
		{
			argv[0] = scriptPath.c_str();
			argv[1] = NULL;
		}
		
		execve(argv[0], const_cast<char* const*>(argv), envArray);
		
		std::cerr << "execve failed for: " << argv[0] << "\n";
		exit(500);
	}
	else
	{
		
		close(stdinPipe[0]);
		close(stdoutPipe[1]);
		
		// Write request body to stdin
		if (!requestBody.empty())
		{
			ssize_t written = write(stdinPipe[1], requestBody.c_str(), requestBody.length());
			if (written < 0)
			{
				std::cerr << "Failed to write to stdin pipe\n";
			}
		}
		close(stdinPipe[1]);
		
		result.output = readWithTimeout(stdoutPipe[0], timeout);
		close(stdoutPipe[0]);
		
		int childStatus;
		waitpid(pid, &childStatus, 0);
		
		if (WIFEXITED(childStatus))
		{
			int exitCode = WEXITSTATUS(childStatus);
			if (exitCode == 0)
			{
				result.statusCode = 200; // Success
			}
			else
			{
				result.statusCode = 500; // Script error
				std::stringstream ss;
				ss << "CGI script exited with code " << exitCode;
				result.error = ss.str();
			}
		}
		else if (WIFSIGNALED(childStatus))
		{
			result.statusCode = 500;
			int signal = WTERMSIG(childStatus);
			std::stringstream ss;
			ss << "CGI script killed by signal " << signal;
			result.error = ss.str();
		}
	}
	
	return result;
}
