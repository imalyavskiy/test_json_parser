// main.cpp : Defines the entry point for the console application.
//

#include "parser_json.h"

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

int main(int argc, char** argv)
{
	json::error error = json::error::ok;

	json::json_parser tokenizer;

	std::string source_file_name;
	if (!read_cmd_line(argc, argv, source_file_name))
		return (int)error;

	std::ifstream source_file(source_file_name, std::ios::in);

	error = json::process(source_file);

	source_file.close();

	return (int)error;
}

