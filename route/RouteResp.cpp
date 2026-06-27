#include "RouteResp.hpp"

RouteResp::RouteResp() : winnerIdx(0) {}

RouteResp::~RouteResp(){}

const std::string& RouteResp::getResponse() const
{
    return response;
}

int RouteResp::locationMatcha(serverConf *conf, ReqContent& cont, int code)
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
        locationConf tmp;
        std::cout << "location error call response 404 Not Found: " << winnerPath << "\n";
        response = respObj.error_response(*conf, tmp, 404);
        return -1;
    }
    else if (code)
    {
        response = respObj.error_response(*conf, conf->locations[winnerIdx], code);
        return -1;
    }
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
                response = respObj.error_response(*conf, location, 404);
                std::cout << "directory or file not found call response 404 Not Found: " << winnerPath << "\n";
                break;
            case EACCES: // permission denied while accessing a component of the path.
                response = respObj.error_response(*conf, location, 403);
                std::cout << "permission denied call response 403 Not Found: " << winnerPath << "\n";
                break;
            case ENAMETOOLONG: // Path name is too long
                response = respObj.error_response(*conf, location, 414);
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

int RouteResp::routeCheck(serverConf *conf, ReqContent& cont, int code)
{
    /* COSTUM ERRORS*/
    if (code && cont.request_target.empty())
    {
        response = respObj.error_response(*conf, conf->locations[winnerIdx], code);
        return -1;
    }

    if (locationMatcha(conf, cont, code) == -1)
        return -1;
    /* CHECK METHODS*/
    if (!conf->locations[winnerIdx].methods.count(cont.method))
    {
        response = respObj.error_response(*conf, conf->locations[winnerIdx], 405);
        std::cout << "method error i must call response 405 Method Not Allowed: " << winnerIdx << "\n";
        return -1;
    }
    /* BODY SIZE */
    if (cont.body.size() > conf->locations[winnerIdx].body_size)
    {
        response = respObj.error_response(*conf, conf->locations[winnerIdx], 413);
        std::cout << "body size error call response for 413 Payload Too Large: " << conf->locations[winnerIdx].body_size << "\n";
        return -1;
    }

    /* CHECK DIRECTIVE */
    if (conf->locations[winnerIdx].http_redire.first)
    {
        response = respObj.redirect(conf->locations[winnerIdx].http_redire.first, conf->locations[winnerIdx].http_redire.second, cont.connection);
        std::cout << "RETURN DIRECTIVE call response for 301 Moved Permanently or 302 Found: " << conf->locations[winnerIdx].http_redire.first << "\n";
        return 0;
    }

    /* FINAL PATH */
    finalPath = conf->locations[winnerIdx].root + cont.request_target;
    
    if (conf->locations[winnerIdx].root[conf->locations[winnerIdx].root.size() - 1] == '/')
        finalPath.erase(conf->locations[winnerIdx].root.size(), 1);
    std::cout << "FINAL PATH: " << finalPath << "\n";

    /* CHECK UPLOAD */
    if (cont.method == "POST" && !conf->locations[winnerIdx].upload_store.empty())
    {
        std::cout << "ITS UPLOAD CALL: " << conf->locations[winnerIdx].upload_store << "\n";
        response = upload.handleUpload(cont, *conf, conf->locations[winnerIdx], cont.connection);
        if (upload.error)
            return -1;
        return 0;
    }
    
    /* DIRECTORY CHECK*/
    struct stat st;

    if (checkDire(finalPath, conf, conf->locations[winnerIdx], &st) == -1)
        return -1;

    if (cont.method == "DELETE")
    {
        std::cout << "ITS A DELETE CALL " << "\n";
        response = respObj.delete_method(*conf, conf->locations[winnerIdx], finalPath, cont.connection);
        if (respObj.error)
            return -1;
        return 0;
    }
    if (cont.method == "GET" && S_ISDIR(st.st_mode)) // if a dire
    {
        std::cout << "ITS A DIRECTORY WITH GET: " << conf->locations[winnerIdx].root << "\n";
        response = respObj.directory(*conf, conf->locations[winnerIdx], finalPath, cont.connection);
        if (respObj.error)
            return -1;
        return 0;
    }

    /* CGI OR STATIC */
    else if (S_ISREG(st.st_mode)) // POST OR DELETE AND FILE
    {
        size_t posDot = finalPath.find_last_of(".");
        if (!conf->locations[winnerIdx].cgi_extension.empty() \
        && posDot != std::string::npos && transLower(finalPath, conf->locations[winnerIdx].cgi_extension, posDot)) // if the cgi extension match the one in request target and a Dot in the last of string
        {
            std::cout << "ITS A CGI CALL: " << finalPath.substr(posDot) << "\n";

            response = Cgi.handleCGIRequest(cont, *conf, conf->locations[winnerIdx], cont.connection);
            if (Cgi.error)
                return -1;
        }
        else if (cont.method == "POST") // respObj.error is post and not cgi 403
        {
            //403 Method Not Allowed
            response = respObj.error_response(*conf, conf->locations[winnerIdx], 403);
            std::cout << "HERE\n";
        }
        else if (cont.method == "GET")// static file
        {
            std::cout << "ITS A STATIC FILE CALL WITH GET: "  << "\n";
            response = respObj.static_file(*conf, conf->locations[winnerIdx], finalPath, cont.connection);
        }
        if (respObj.error)
            return -1;
        return 0;
    }
    /* POST AND FOLDER AND NOT UPLOAD */
    std::cout << "403 Forbidden" << "\n";
    response = respObj.error_response(*conf, conf->locations[winnerIdx], 403);
    // 403 Forbidden
    return -1;
    
}