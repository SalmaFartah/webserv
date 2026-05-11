#include "parse.hpp"

conf::conf() {};
conf::~conf() {};


void conf::fillWords(std::ifstream& confWords, int c)
{
    std::string word;

    word.clear();
    while (c != EOF && c != '{' && c != '}' && c != ';' \
    && !std::isspace(c) && c != '#')
    {
        word.push_back(c);
        c = confWords.get();
    }
    if (!word.empty())
        vecToken.push_back(std::make_pair(WORD, word));
    if (c != EOF)
        confWords.unget();
}

void conf::read_file(std::ifstream& conf)
{
    std::string line;
    int c;
    while ((c = conf.get()) != EOF)
    {
        if (std::isspace(c))
            continue;
        else if (c == '#')
            std::getline(conf, line);
        else if (c == '{')
            vecToken.push_back(std::make_pair(OPEND_BC, "{"));
        else if (c == '}')
            vecToken.push_back(std::make_pair(CLOSED_BC, "}"));
        else if (c == ';')
            vecToken.push_back(std::make_pair(SEMI_COL, ";"));
        else
            fillWords(conf, c);
    }

}

// void conf::print_tokenz()
// {
//     for (size_t i = 0; i < vecToken.size(); i++)
//     {
//         std::cout << "TYPE: " << vecToken[i].first << " VALUE: " << vecToken[i].second << std::endl;       
//     }
    
// }