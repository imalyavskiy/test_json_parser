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

void print_symbol(const unsigned char& c)
{
	std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::resetiosflags(std::ios_base::basefield);
	
	if (c <= 0x1F)
		std::cout << "    ";
	else
		std::cout << " \'" << c << "\'";
	
	std::cout << std::endl;
}

json::result
process(/*const */std::istream& input)
{
	char c = 0;
	json::parser::ptr p = json::create_array_parser();
	if (!p)
		return result::e_fatal;

	while (input >> std::noskipws >> c)
	{
		print_symbol(c);

		if (json::result::s_ok > p->step(c, (int)input.tellg() - 1))
		{
			std::cout << "Error while parsing: reason \'" << c << "\'" << std::endl;
			break;
		}
	}

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

