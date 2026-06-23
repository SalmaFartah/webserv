#include "RouteResp.hpp"

RouteResp::RouteResp() : winnerIdx(0) {}

RouteResp::~RouteResp(){}

const std::string& RouteResp::getResponse() const
{
    return response;
}

int RouteResp::locationMatcha(serverConf *conf, ReqContent& cont)
{
    for (size_t i = 0; i < conf->locations.size(); i++)
    {
        if (cont.request_target.find(conf->locations[i].path) == 0 && conf->locations[i].path.size() > winnerPath.size())
        {
            winnerPath = conf->locations[i].path;
            winnerIdx = i;
        }
    }
    if (winnerPath.empty())
    {
        std::cout << "location error call response 404 Not Found: " << winnerPath << "\n";
        return -1;
    }
    else
        std::cout << "test matcha location: " << winnerPath << "\n";
    return 0;
    
}
int RouteResp::checkDire(std::string &fullPath, serverConf *conf, locationConf& location, struct stat *st)
{
    (void)conf;
    (void)location;
    if (stat(fullPath.c_str(), st) == -1)
    {
        switch (errno)
        {
            case ENOENT: case ENOTDIR: // the file or directory does not exist at all or component of the path is not a directory
                std::cout << "directory or file not found call response 404 Not Found: " << winnerPath << "\n";
                break;
            case EACCES: // permission denied while accessing a component of the path.
                std::cout << "permission denied call response 403 Not Found: " << winnerPath << "\n";
                break;
            case ENAMETOOLONG: // Path name is too long
                std::cout << "Path name is too long call response 414 Not Found: " << winnerPath << "\n";
                break;
            // response = take response and set it;
        }
        return -1;
    }
    return 1;
}

int RouteResp::transLower(std::string& strPath, std::string& strExt, size_t posDot)
{
    std::string lowerPath(strPath.size() - posDot, '\0');
    std::string lowerExten(strExt.size() - strExt.find_last_of("."), '\0');

    std::transform(strPath.begin() + posDot, strPath.end(), lowerPath.begin(), tolower);
    std::transform(strExt.begin() + strExt.find_last_of("."), strExt.end(), lowerExten.begin(), tolower);

    // std::cout << "final path after dot: " << lowerPath << "\n";
    // std::cout << "cgi extension after dot: " << strExt << "\n";

    if (lowerPath == lowerExten)
        return 1;
    return 0;
}

int RouteResp::routeCheck(serverConf *conf, ReqContent& cont)
{
    
    if (locationMatcha(conf, cont) == -1)
        return -1;

    /* CHECK METHODS*/
    if (!conf->locations[winnerIdx].methods.count(cont.method))
        std::cout << "method error i must call response 405 Method Not Allowed: " << winnerIdx << "\n";
    else
        std::cout << "method found it is: " << cont.method << "\n";

    /* BODY SIZE */
    if (cont.body.size() > conf->locations[winnerIdx].body_size)
        std::cout << "body size error call response for 413 Payload Too Large: " << conf->locations[winnerIdx].body_size << "\n";
    else
        std::cout << "max body-size found is: " << conf->locations[winnerIdx].body_size << "\n";

    /* RETURN DIRECTIVE */
    if (conf->locations[winnerIdx].http_redire.first)
    {
        std::cout << "RETURN DIRECTIVE call response for 301 Moved Permanently or 302 Found: " << conf->locations[winnerIdx].http_redire.first << "\n";
        return 0;
    }

    /* FINAL PATH */
    finalPath = conf->locations[winnerIdx].root + cont.request_target;
    
    if (conf->locations[winnerIdx].root.back() == '/')
        finalPath.erase(conf->locations[winnerIdx].root.size(), 1);
    std::cout << "FINAL PATH: " << finalPath << "\n";

    /* CHECK UPLOAD */
    if (cont.method == "POST" && !conf->locations[winnerIdx].upload_store.empty())
    {
        std::cout << "ITS UPLOAD CALL: " << conf->locations[winnerIdx].upload_store << "\n";
        // response = directory(std::string &fullPath, serverConf *conf, locationConf& location, cont.connection);
        // if (error)
        //     return -1;
        return 0;
    }
    
    /* DIRECTORY CHECK*/
    struct stat st;

    if (checkDire(finalPath, conf, conf->locations[winnerIdx], &st) == -1)
        return -1;

    if (cont.method == "GET" && S_ISDIR(st.st_mode)) // if a dire
    {
        std::cout << "ITS A DIRECTORY WITH GET: " << winnerPath << "\n";
        // response = directory(std::string &fullPath, serverConf *conf, locationConf& location, cont.connection);
        // if (error)
        //     return -1;
        return 0;
    }

    /* CGI OR STATIC */
    else if (S_ISREG(st.st_mode)) // POST OR DELETE AND FILE
    {
        size_t posDot = finalPath.find_last_of(".");
        if (!conf->locations[winnerIdx].cgi_extension.empty() && posDot != std::string::npos && transLower(finalPath, conf->locations[winnerIdx].cgi_extension, posDot)) // if the cgi extension match the one in request target and a Dot in the last of string
        {
            // response = cgiCall(conf, conf->locations[winnerIdx]);
            // if (error)
            //     return -1;
            std::cout << "ITS A CGI CALL: " << finalPath.substr(posDot) << "\n";
        }
        // else if (cont.method == "POST") // error is post and not cgi 405
        // {
        //     //405 Method Not Allowed
        //     return -1;
        // }
        else if (cont.method == "GET")// static file
        {
            std::cout << "ITS A STATIC FILE CALL WITH GET: "  << "\n";
            // response = directory(std::string &fullPath, serverConf *conf, locationConf& location, cont.connection);
            // if (error)
            //     return -1;     
        }
        else if (cont.method == "DELETE")
        {
            std::cout << "ITS A DELETE CALL: "  << "\n";
            // response = directory(std::string &fullPath, serverConf *conf, locationConf& location, cont.connection);
            // if (error)
            //     return -1; 
        }
    }
    /* POST OR DELETE AND FOLDER || POST AND NOT CGI*/
    std::cout << "405 Method Not Allowed: " << "\n";
    // 405 Method Not Allowed
    return -1;
    
}