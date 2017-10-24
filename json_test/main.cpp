// main.cpp : Defines the entry point for the console application.
//
#include "../json_lib/json_lib.h"

typedef json::result result;

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

json::result
process(/*const */std::istream& input)
{
	char c = 0;
	json::parser::ptr p = json::create_number_parser();
	if (!p)
		return result::e_fatal;

	while (input >> std::noskipws >> c)
	{
		std::cout << c;

		if (json::result::s_ok > p->step(c, (int)input.tellg() - 1))
		{
			std::cout << "Error while parsing" << std::endl;
			break;
		}
	}

	std::cout << std::endl;

	return json::result::s_ok;
}

json::result
process(const std::string& input)
{
	std::stringstream sstr;
	sstr.str(input);
	return process(sstr);
}

int main(int argc, char** argv)
{
	json::result res = json::result::s_ok;

	std::string source_file_name;
	if (!read_cmd_line(argc, argv, source_file_name))
		return (int)res;

	std::ifstream source_file(source_file_name, std::ios::in);

	res = process(source_file);

	source_file.close();

	return (int)res;
}

