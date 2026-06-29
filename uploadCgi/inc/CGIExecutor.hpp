#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include <map>
#include <string>
#include "../../parse_config/inc/Fill.hpp"

class CGIExecutor
{
    public:
        void executeCGI(const std::string& scriptPath, const std::string& cgi_pass, const std::map<std::string, std::string>& envVars, const std::string& requestBody, CGIResult& CgiRes);

    private:
        static char** mapToEnvArray(const std::map<std::string, std::string>& envVars);
        static void freeEnvArray(char** envArray, size_t count);
        static std::string readWithTimeout(int fd, int childid, size_t timeout);
};

#endif