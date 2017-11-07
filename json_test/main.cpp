// main.cpp : Defines the entry point for the console application.
//
#include "../json_lib/json_lib.h"

typedef imalyavskiy::json::result_t result_t;
using json = imalyavskiy::json;

bool read_cmd_line(int argc, char** argv, std::string& source_file)
{
	auto print_usage = []() { std::cout << "USAGE: The only parameter is the text source file path." << std::endl; };

	if (argc < 2)
		std::cout << "[ERROR] Not enough parameters!" << std::endl, print_usage(), exit(1);

	if (argc > 2)
		std::cout << "[ERROR] Too many parameters!" << std::endl, print_usage(), exit(1);

	std::ifstream src(argv[1]);
	if (!src.is_open())
		std::cout << "[ERROR] Check does the file exist." << std::endl, print_usage(), exit(1);
	else
		src.close();

	source_file = argv[1];

	return true;
}

int parser_main(int argc, char** argv)
{
	json::result_t res = json::result_t::s_ok;

	std::string source_file_name;
	if (!read_cmd_line(argc, argv, source_file_name))
		return (int)res;

	std::ifstream source_file(source_file_name, std::ios::in);
	json::object jsobj;
	res = json::parse(source_file, jsobj);

    std::cout << jsobj.str() << std::endl;

	source_file.close();

	return (int)res;
}

/*
{
	"glossary": {
		"title": "example glossary",
		"GlossDiv" : {
			"title": "S",
			"GlossList" : {
				"GlossEntry": {
					"ID": "SGML",
					"SortAs" : "SGML",
					"GlossTerm" : "Standard Generalized Markup Language",
					"Acronym" : "SGML",
					"Abbrev" : "ISO 8879:1986",
					"GlossDef" : {
						"para": "A meta-markup language, used to create markup languages such as DocBook.",
						"GlossSeeAlso" : [
							"GML", 
							"XML"
						]
					},
					"GlossSee" : "markup"
				}
			}
		}
	}
}
*/
const std::string& json_data_structure_2(std::string& str = std::string())
{
	using obj = json::object;
	using arr = json::array;

	auto glossary = obj
	{
		{ "title", "example glossary"},
		{ "GlossDiv", obj{
			{ "title", "S"},
			{ "GlossList", obj{
				{ "GlossEntry", obj{
					{ "ID", "SGML"},
					{ "SortAs", "SGML"},
					{ "GlossTerm", "Standard Generalized Markup Language"},
					{ "Acronym", "SGML"},
					{ "Abbrev", "ISO 8879:1986"},
					{ "GlossDef", obj{
						{ "GlossDef", "A meta-markup language, used to create markup languages such as DocBook."},
						{ "GlossSeeAlso", arr{ 
							"GML",
							"XML",
							true,
							false,
							nullptr,
							(int64_t)-1,
							(int64_t)100
						}}
					}}
				}}
			}}
		}}
	};

	std::cout << glossary.str() << std::endl;

	return str;
}

const std::string& json_data_structure_1(std::string& str = std::string())
{
	json::array GlossSeeAlso;
	GlossSeeAlso.push_back("GML");
	GlossSeeAlso.push_back("XML");
	GlossSeeAlso.push_back(true);
	GlossSeeAlso.push_back(false);
	GlossSeeAlso.push_back(nullptr);
	GlossSeeAlso.push_back((int64_t)-1);
	GlossSeeAlso.push_back((int64_t)100);

	json::object GlossDef;
	GlossDef["GlossDef"]        = std::string("A meta-markup language, used to create markup languages such as DocBook.");
	GlossDef["GlossSeeAlso"]    = GlossSeeAlso;

	json::object GlossEntry;
	GlossEntry["ID"]            = std::string("SGML");
	GlossEntry["SortAs"]        = std::string("SGML");
	GlossEntry["GlossTerm"]     = std::string("Standard Generalized Markup Language");
	GlossEntry["Acronym"]       = std::string("SGML");
	GlossEntry["Abbrev"]        = std::string("ISO 8879:1986");
	GlossEntry["GlossDef"]      = GlossDef;

	json::object GlossList;
	GlossList["GlossEntry"]     = GlossEntry;

	json::object GlossDiv;
	GlossDiv["title"]           = std::string("S");
	GlossDiv["GlossList"]       = GlossList;

	json::object glossary;
	glossary["title"]           = std::string("example glossary");
	glossary["GlossDiv"]        = GlossDiv;

	std::cout << glossary.str() << std::endl;

	return str;
}

int main(int argc, char** argv)
{
    bool result = json_data_structure_1() == json_data_structure_2();
    assert(true == result);

	parser_main(argc, argv);

	return 1;
};


