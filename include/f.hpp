#pragma once

#include <string>
#include <vector>

typedef struct
{
    std::string path; // location /path/
    std::vector<std::string> methods;// POST/GET/DELETE
    std::pair<int, std::string> http_redire; // return direct
    std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    std::vector<std::string> index; // index index.php index.html;
    std::string upload_store; // path where the client post smth
}   locationConf;

typedef struct
{
    std::string serverName;
    std::vector<std::pair<std::string, int>> listen; // default localhost:80
    std::string error_page;
    size_t max_body_sz;
    std::vector<locationConf> locations;
}   serverConf;
