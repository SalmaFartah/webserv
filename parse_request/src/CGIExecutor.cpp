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

    char** envArray = new char*[envVars.size() + 1];
    size_t i = 0;
    
    for (std::map<std::string, std::string>::const_iterator it = envVars.begin();
         it != envVars.end(); ++it) {
        std::string envStr = it->first + "=" + it->second;
        envArray[i] = new char[envStr.length() + 1];
        std::strcpy(envArray[i], envStr.c_str());
        ++i;
    }
    envArray[envVars.size()] = NULL;  // ✅ NULL au lieu de nullptr
    
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

std::string CGIExecutor::readWithTimeout(int fd, size_t timeout)
{
    std::string result;
    char buffer[4096];
    fd_set readFds;
    struct timeval tv;
    
    while (true) {
        FD_ZERO(&readFds);
        FD_SET(fd, &readFds);
        
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        
        
        int selectResult = select(fd + 1, &readFds, NULL, NULL,
                                   timeout > 0 ? &tv : NULL);
        
        if (selectResult <= 0)
            break;
        
        if (FD_ISSET(fd, &readFds)) {
            ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0)
                break;
            
            result.append(buffer, bytesRead);
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
) {
    CGIResult result;
    
    int stdinPipe[2];
    int stdoutPipe[2];
    
    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
        result.statusCode = 500;
        result.error = "Failed to create pipes";
        return result;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        result.statusCode = 500;
        result.error = "Failed to fork child process";
        return result;
    }
    
    if (pid == 0) {
        // Processus enfant
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        
        char** envArray = mapToEnvArray(envVars);
        
        const char* argv[3];
        if (!interpreterPath.empty()) {
            argv[0] = interpreterPath.c_str();
            argv[1] = scriptPath.c_str();
            argv[2] = NULL;
        } else {
            argv[0] = scriptPath.c_str();
            argv[1] = NULL;
        }
        
        execve(argv[0], const_cast<char* const*>(argv), envArray);
        
        freeEnvArray(envArray, envVars.size());
        std::cerr << "execve failed for: " << argv[0] << "\n";
        exit(500);
    }
    
    // Processus parent
    close(stdinPipe[0]);
    close(stdoutPipe[1]);
    
    if (!requestBody.empty()) {
        write(stdinPipe[1], requestBody.c_str(), requestBody.size());
    }
    close(stdinPipe[1]);
    
    result.output = readWithTimeout(stdoutPipe[0], timeout);
    close(stdoutPipe[0]);
    
    int childStatus;
    waitpid(pid, &childStatus, 0);
    
    if (WIFEXITED(childStatus) && WEXITSTATUS(childStatus) == 0) {
        result.statusCode = 200;
    } else {
        result.statusCode = 500;
        result.error = "CGI script failed";
        if (WIFEXITED(childStatus)) {
            std::stringstream ss;
            ss << " (exit code: " << WEXITSTATUS(childStatus) << ")";
            result.error += ss.str();
        } else if (WIFSIGNALED(childStatus)) {
            std::stringstream ss;
            ss << " (signal: " << WTERMSIG(childStatus) << ")";
            result.error += ss.str();
        }
    }
    
    return result;
}