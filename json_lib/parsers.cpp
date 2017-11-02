#include "parsers.h"

using namespace json;

json::parser::ptr
json::create_object_parser()
{
	return json::parser::ptr(new object_parser());
}

json::parser::ptr
json::create_array_parser()
{
	return json::parser::ptr(new array_parser());
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

json::parser::ptr
json::create_value_parser()
{
	return json::parser::ptr(new value_parser());
}


object_t::object_t(std::initializer_list<std::pair<std::string, value>> l)
{
	for (auto arg : l)
		insert(arg);
}

const std::string
object_t::str(std::stringstream& str)
{
	// leading curly brace
	str << "{";

	for (auto it = begin(); it != end(); ++it)
	{
		const bool last = (--end() == it);

		// key
		str << "\"" << it->first << "\":";

		// value
		switch ((vt)it->second.index())
		{
		case vt::t_string:
			str << "\"" << std::get<std::string>(it->second) << "\"";
			break;
		case vt::t_object:
			std::get<object_t>(it->second).str(str);
			break;
		case vt::t_array:
			std::get<array_t>(it->second).str(str);
			break;
		case vt::t_int64:
			str << std::get<int64_t>(it->second);
			break;
		case vt::t_floatingpt:
			str << std::scientific << std::get<double>(it->second);
			break;
		case vt::t_boolean:
			str << (std::get<bool>(it->second) ? "true" : "false");
			break;
		case vt::t_null:
			str << "null";
			break;
		default: // unknown(i.e. not mentioned) type
			assert(0);
			break;
		}

		// comma if not the last one
		str << (last ? "" : ",");
	}

	// trailing curly brace
	str << "}";

	return str.str();
}


array_t::array_t(std::initializer_list<value> l)
{
	for (auto arg : l)
		push_back(arg);
}

const std::string
array_t::str(std::stringstream& str)
{
	// leading curly brace
	str << "[";

	for (auto it = begin(); it != end(); ++it)
	{
		const bool last = (--end() == it);

		// value
		switch ((vt)it->index())
		{
		case vt::t_string:
			str << "\"" << std::get<std::string>(*it) << "\"";
			break;
		case vt::t_object:
			std::get<object_t>(*it).str(str);
			break;
		case vt::t_array:
			std::get<array_t>(*it).str(str);
			break;
		case vt::t_int64:
			str << std::get<int64_t>(*it);
			break;
		case vt::t_floatingpt:
			str << std::scientific << std::get<double>(*it);
			break;
		case vt::t_boolean:
			str << ((std::get<bool>(*it) ? "true" : "false"));
			break;
		case vt::t_null:
			str << "null";
			break;
		default:
			assert(0);
			break;
		}

		// comma if not the last one
		str << (last ? "" : ",");
	}

	// trailing curly brace
	str << "]";

	return str.str();
}