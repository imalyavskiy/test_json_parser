#include "parsers.h"

using namespace json;

value_parser::value_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::other,				{ state_t::read,	BIND(value_parser::on_data)		} },
		} },
		{ state_t::read,		{	{ event_t::other,				{ state_t::read,	BIND(value_parser::on_data)		} },
		} },
		{ state_t::done,		{	{ event_t::other,				{ state_t::failure,	BIND(value_parser::on_fail)		} },
		} },
		{ state_t::failure,		{	{ event_t::other,				{ state_t::failure,	BIND(value_parser::on_fail)		} },
		} },
	}
{
}

value_parser::~value_parser()
{
}

result
value_parser::step(const char& c, const int pos)
{
	result res = parser_impl::step(c, pos);
	if (state::get() == state_t::read && (result::s_done == res || result::s_done_rpt == res))
		state::set(state_t::done); // forced state change
	return res;
}


result 
value_parser::on_data(const unsigned char& c, const int pos)
{
	result res = result::e_fatal;
	uint8_t parsers_in_work = 0;

	if (parsing_unit.empty())
	{
		parsing_unit.push_back(ParserItem_t(true, create_null_parser()));
		parsing_unit.push_back(ParserItem_t(true, create_bool_parser()));
		parsing_unit.push_back(ParserItem_t(true, create_string_parser()));
		parsing_unit.push_back(ParserItem_t(true, create_number_parser()));
		parsing_unit.push_back(ParserItem_t(true, create_array_parser()));
		parsing_unit.push_back(ParserItem_t(true, create_object_parser()));
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

	if (0 == parsers_in_work)
		res = result::e_unexpected;

	return res;
}

result 
value_parser::on_done(const unsigned char& c, const int pos)
{
	return result::s_done;
}

result 
value_parser::on_fail(const unsigned char& c, const int pos)
{
	return result::e_unexpected;
}

void 
value_parser::reset()
{
	std::cout << ">>> begin reset" << std::endl;
	state::set(state_t::initial);
	for (ParserItem_t& p : parsing_unit)
		p.first = true, p.second->reset();
	std::cout << "<<< end reset" << std::endl;
}

value_parser::event_t 
value_parser::to_event(const char& c) const
{
	return event_t::other;
}
