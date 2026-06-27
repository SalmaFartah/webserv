#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include <map>
#include <string>

class CGIExecutor
{
    public:
        struct CGIResult
        {
            int statusCode;
            std::string output;
        };
        
        CGIResult executeCGI(const std::string& scriptPath, const std::string& cgi_pass, const std::map<std::string, std::string>& envVars, const std::string& requestBody, size_t timeout);

    private:
        static char** mapToEnvArray(const std::map<std::string, std::string>& envVars);
        static void freeEnvArray(char** envArray, size_t count);
        static std::string readWithTimeout(int fd, int childid, size_t timeout);
};

#endif