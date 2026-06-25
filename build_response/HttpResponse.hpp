#pragma once

#include "../parse_config/inc/Fill.hpp"
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

class HttpResponse
{
		std::string methods;
		std::string url;
		std::map<std::string, std::string> MIME_table;

		std::string getErrorPage(int, const std::string&);
		std::string getReasonPhrase(int);
		void initMimeTable();
		std::string build(int, const std::string&, const std::string&, bool);
	public:
		HttpResponse();
		bool error;
		std::string redirect(int, const std::string&, bool);
		std::string delete_method(serverConf&, locationConf&, const std::string&, bool);
		std::string static_file(serverConf&, locationConf&, const std::string&, bool);
		std::string directory(serverConf&, locationConf&, std::string, bool);
		std::string error_response(serverConf&, locationConf&, int);
		~HttpResponse();
};


/*
<a href="filename">filename</a>
 │  │       │          │      │
 │  │       │          │      └─ closing tag, marks the end of the link
 │  │       │          └─ the visible text shown on the page (what you see and click)
 │  │       └─ the actual destination — where clicking takes you
 │  └─ "href" means "hyperlink reference" — the attribute that holds the destination
 └─ "a" means "anchor" — the HTML tag name for a clickable link
*/

// file.txt
// h.jpeg