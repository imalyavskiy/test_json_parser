#include "parsers.h"

using namespace json;

value_parser::value_parser()
	: m_state_table
	{
		{ read_state_t::initial,	{	{ symbol_t::other,				{ read_state_t::read,	BIND(value_parser::on_data)		} },
		} },
		{ read_state_t::read,		{	{ symbol_t::other,				{ read_state_t::read,	BIND(value_parser::on_data)		} },
		} },
	}
{
	reset();
}

value_parser::~value_parser()
{

}

result 
value_parser::on_data(const unsigned char& c, const int pos)
{
	result res = result::e_fatal;
	uint8_t parsers_in_work = 0;

	if (parsing_unit.empty())
	{
		parsing_unit.push_back(std::pair<bool, parser::ptr>(true, create_null_parser()));
		parsing_unit.push_back(std::pair<bool, parser::ptr>(true, create_bool_parser()));
		parsing_unit.push_back(std::pair<bool, parser::ptr>(true, create_string_parser()));
		parsing_unit.push_back(std::pair<bool, parser::ptr>(true, create_number_parser()));
		parsing_unit.push_back(std::pair<bool, parser::ptr>(true, create_array_parser()));
		parsing_unit.push_back(std::pair<bool, parser::ptr>(true, create_object_parser()));
	}
	
	for(std::pair<bool, parser::ptr>& p : parsing_unit)
	{
		if (true == p.first)
		{
			result local_res = p.second->step(c, pos);
			if (json_failed(local_res))
				p.first = false;
			else if (json_succeded(local_res) && (json_failed(res) || local_res < res))
				res = local_res, parsers_in_work += 1;
		}
	}

	assert(parsers_in_work);

	return res;
}

result 
value_parser::on_done(const unsigned char& c, const int pos)
{
	return result::s_done;
}

void 
value_parser::reset()
{
	state::set(read_state_t::initial);
	for (std::pair<bool, parser::ptr>& p : parsing_unit)
		p.first = true, p.second->reset();
}
