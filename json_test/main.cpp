// main.cpp : Defines the entry point for the console application.
//
#include "../json_lib/json_lib.h"

typedef json::result_t result_t;

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

void print_symbol(const unsigned char& c)
{
	std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::resetiosflags(std::ios_base::basefield);
	
	if (c <= 0x1F)
		std::cout << "    ";
	else
		std::cout << " \'" << c << "\'";
	
	std::cout << std::endl;
}

json::result_t
process(/*const */std::istream& input, json::object_t<>& jsobj)
{
	char c = 0;
	json::result_t result = json::result_t::s_ok;

	json::parser::ptr p = json::create();
	if (!p)
		return result_t::e_fatal;

	int pos = 0;

	while (input >> std::noskipws >> c || result != json::result_t::s_done)
	{
		//print_symbol(c);

		pos = (int)input.tellg() - 1;

		result = p->putchar(c, pos);

		if (json::result_t::s_ok > result)
		{
			std::cout << "Error while parsing: reason \'" << c << "\' at " << pos << std::endl;
			break;
		}

		if(json::result_t::s_done == result)
		{
			//std::cout << "Success" << std::endl;

			json::value val = p->get();
			json::object_t<> obj = std::get<json::object_t<>>(val);

			std::cout << obj.str() << std::endl;

			break;
		}
	}

	return result;
}

json::result_t
process(const std::string& input, json::object_t<>& jsobj)
{
	std::stringstream sstr;
	sstr.str(input);
	return process(sstr, jsobj);
}

int parser_main(int argc, char** argv)
{
	json::result_t res = json::result_t::s_ok;

	std::string source_file_name;
	if (!read_cmd_line(argc, argv, source_file_name))
		return (int)res;

	std::ifstream source_file(source_file_name, std::ios::in);
	json::object_t<> jsobj;
	res = process(source_file, jsobj);

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
const std::string& json_data_structure_2(std::string& str)
{
	using obj = json::object_t<>;
	using arr = json::array_t<>;

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

const std::string& json_data_structure_1(std::string& str)
{
	json::array_t<> GlossSeeAlso;
	GlossSeeAlso.push_back("GML");
	GlossSeeAlso.push_back("XML");
	GlossSeeAlso.push_back(true);
	GlossSeeAlso.push_back(false);
	GlossSeeAlso.push_back(nullptr);
	GlossSeeAlso.push_back((int64_t)-1);
	GlossSeeAlso.push_back((int64_t)100);

	json::object_t<> GlossDef;
	GlossDef["GlossDef"] = std::string("A meta-markup language, used to create markup languages such as DocBook.");
	GlossDef["GlossSeeAlso"] = GlossSeeAlso;

	json::object_t<> GlossEntry;
	GlossEntry["ID"] = std::string("SGML");
	GlossEntry["SortAs"] = std::string("SGML");
	GlossEntry["GlossTerm"] = std::string("Standard Generalized Markup Language");
	GlossEntry["Acronym"] = std::string("SGML");
	GlossEntry["Abbrev"] = std::string("ISO 8879:1986");
	GlossEntry["GlossDef"] = GlossDef;

	json::object_t<> GlossList;
	GlossList["GlossEntry"] = GlossEntry;

	json::object_t<> GlossDiv;
	GlossDiv["title"] = std::string("S");
	GlossDiv["GlossList"] = GlossList;

	json::object_t<> glossary;
	glossary["title"] = std::string("example glossary");
	glossary["GlossDiv"] = GlossDiv;

	std::cout << glossary.str() << std::endl;

	return str;
}



int json_data_structure_main(int argc, char** argv)
{
	std::string str1;
	std::string str2;
	
	json_data_structure_2(str1);
	json_data_structure_2(str2);

	const bool result = (str1 == str2);
	assert(result);

	return result;
};

int main(int argc, char** argv)
{
	parser_main(argc, argv);

	return 1;
};


