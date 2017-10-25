#include "parsers.h"

using namespace json;

json::parser::ptr 
json::create_object_parser()
{
	return json::parser::ptr();
}

json::parser::ptr 
json::create_array_parser()
{
	return json::parser::ptr();
}

json::parser::ptr 
json::create_number_parser()
{
	return json::parser::ptr(new number_parser());
}

json::parser::ptr 
json::create_string_parser()
{
	return json::parser::ptr(new string_parser());
}

json::parser::ptr 
json::create_null_parser()
{
	return json::parser::ptr(new null_parser());
}

json::parser::ptr 
json::create_bool_parser() 
{
	return json::parser::ptr(new bool_parser());
}