#include "parsers.h"

using namespace json;

value_parser::value_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::symbol,				{ state_t::read,	BIND(value_parser::on_data)	} },
		} },
		{ state_t::read,		{	{ event_t::symbol,				{ state_t::read,	BIND(value_parser::on_data)	} },
									{ event_t::val_done,			{ state_t::done,	BIND(value_parser::on_done)	} },
		} },
		{ state_t::done,		{	{ event_t::symbol,				{ state_t::failure,	BIND(value_parser::on_fail)	} },
		} },
		{ state_t::failure,		{	{ event_t::symbol,				{ state_t::failure,	BIND(value_parser::on_fail)	} },
		} },
	}
{
}

value_parser::~value_parser()
{
}

result_t
value_parser::putchar(const char& c, const int pos)
{
	result_t r = parser_impl::step(to_event(c), c, pos);
	
	if (state::get() == state_t::read && (result_t::s_done == r || result_t::s_done_rpt == r))
	{
		result_t new_r = parser_impl::step(to_event(r), c, pos);
		assert(result_t::s_done == new_r);
		return  r != new_r ? r : new_r;
	}

	return r;
}


result_t 
value_parser::on_data(const unsigned char& c, const int pos)
{
	result_t res = result_t::e_fatal;
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
			result_t local_res = p.second->putchar(c, pos);

			if (json_failed(local_res))
				p.first = false;
			else if (json_succeded(local_res) && (json_failed(res) || local_res < res))
				res = local_res, parsers_in_work += 1;
		}
	}

	if (0 == parsers_in_work)
		res = result_t::e_unexpected;

	return res;
}

result_t 
value_parser::on_done(const unsigned char& c, const int pos)
{
	return result_t::s_done;
}

result_t 
value_parser::on_fail(const unsigned char& c, const int pos)
{
	return result_t::e_unexpected;
}

void 
value_parser::reset()
{
#ifdef _DEBUG
	std::cout << ">>> begin reset" << std::endl;
#endif // _DEBUG

	state::set(state_t::initial);
	for (ParserItem_t& p : parsing_unit)
		p.first = true, p.second->reset();

#ifdef _DEBUG
	std::cout << "<<< end reset" << std::endl;
#endif // _DEBUG
}

value_parser::event_t 
value_parser::to_event(const char& c) const
{
	return event_t::symbol;
}

value_parser::event_t
value_parser::to_event(const result_t& c) const
{
	switch (state::get())
	{
	case state_t::read:
		if (result_t::s_done == c || result_t::s_done_rpt == c)
			return event_t::val_done;
		break;
	}

	return event_t::nothing;
}

