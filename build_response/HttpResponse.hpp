#pragma once

#include "../parse_config/inc/Fill.hpp"
#include <map>
#include <fstream>

class HttpResponse
{
		std::map<std::string, std::string> MIME_table;
	public:
		HttpResponse();
		std::string error_response(int, serverConf&, locationConf&);
		std::string getErrorPage(int, const std::string&);
		std::string getReasonPhrase(int);
		std::string get_errbody(int, const std::string&, serverConf&, locationConf&, std::string&);
		void initMimeTable();
		~HttpResponse();
};
