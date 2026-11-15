#include "../inc/CGIExecutor.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <sys/select.h>
#include <iostream>
#include <signal.h>
#include <sstream>

char** CGIExecutor::mapToEnvArray(const std::map<std::string, std::string>& envVars)
{

    char** envArray = new char*[envVars.size() + 1];
    size_t i = 0;
    
    for (std::map<std::string, std::string>::const_iterator it = envVars.begin(); it != envVars.end(); ++it) 
    {
        std::string envStr = it->first + "=" + it->second;
        envArray[i] = new char[envStr.length() + 1];
        std::strcpy(envArray[i], envStr.c_str());
        ++i;
    }
    envArray[envVars.size()] = NULL;

    return envArray;
}

void CGIExecutor::freeEnvArray(char** envArray, size_t count)
{
    if (!envArray)
        return;
    
    for (size_t i = 0; i < count; ++i) {
        delete[] envArray[i];
    }
    delete[] envArray;
}

void CGIExecutor::executeCGI(const std::string& scriptPath, const std::string& cgi_pass, \
const std::map<std::string, std::string>& envVars, const std::string& requestBody, CGIResult& CgiRes)
{
    int stdinPipe[2];
    int stdoutPipe[2];
    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1)
    {
        CgiRes.statusCode = 500;
        return ;
    }
    pid_t pid = fork();

    if (pid == -1)
    {
        CgiRes.statusCode = 500;
        return ;
    }
    
    if (pid == 0)
    {
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        
        char** envArray = mapToEnvArray(envVars);
        
        const char* argv[3];
       
        argv[0] = cgi_pass.c_str();
        argv[1] = scriptPath.c_str();
        argv[2] = NULL;
        
        execve(argv[0], const_cast<char* const*>(argv), envArray);
        
        freeEnvArray(envArray, envVars.size());
        std::cerr << "execve failed for: " << argv[0] << "\n";
        exit(500);
    }
    // Processus parent
    close(stdinPipe[0]);
    close(stdoutPipe[1]);
    CgiRes.stdinPipe = stdinPipe[1];
    CgiRes.stdoutPipe = stdoutPipe[0];
    CgiRes.pidChild = pid;
    CgiRes.body = requestBody;
    CgiRes.ofssetCgi = 0;
}