#include "parsers.h"

using namespace json;

array_parser::array_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::left_square_bracket,		{ state_t::before_val,	BIND(array_parser::on_before_value)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(array_parser::on_fail)			} },
		} },
 		{ state_t::before_val,	{	{ event_t::right_square_bracket,	{ state_t::done,		BIND(array_parser::on_done)			} },
 									{ event_t::symbol,					{ state_t::in_value,	BIND(array_parser::on_value)		} },
 		} },
		{ state_t::after_val,	{	{ event_t::right_square_bracket,	{ state_t::done,		BIND(array_parser::on_done)			} },
									{ event_t::comma,					{ state_t::before_val,	BIND(array_parser::on_before_value)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(array_parser::on_fail)			} },
		} },
 		{ state_t::in_value,	{	{ event_t::symbol,					{ state_t::in_value,	BIND(array_parser::on_value)		} },
 		} },
		{ state_t::done,		{	{ event_t::symbol,					{ state_t::failure,		BIND(array_parser::on_fail)			} },
		} },
		{ state_t::failure,		{	{ event_t::symbol,					{ state_t::failure,		BIND(array_parser::on_fail)			} },
		} },
	}
	, m_value_parser(create_value_parser())
{
}

array_parser::~array_parser()
{
}

result_t
array_parser::on_before_value(const unsigned char& c, const int pos)
{
	m_value_parser->reset();
	return result_t::s_need_more;
}

result_t
array_parser::on_after_value(const unsigned& c, const int pos)
{
	return result_t::s_need_more;
}

result_t 
array_parser::on_value(const unsigned& c, const int pos)
{
	return m_value_parser->putchar(c, pos);
}

result_t
array_parser::on_done(const unsigned char& c, const int pos)
{
	return result_t::s_done;
}

result_t 
array_parser::on_fail(const unsigned char& c, const int pos)
{
	return result_t::e_unexpected;
}

void 
array_parser::reset()
{
	std::cout << ">>> begin reset" << std::endl;
	state::set(state_t::initial);
	m_value_parser->reset();
	std::cout << ">>> end reset" << std::endl;
}

result_t 
array_parser::putchar(const char& c, const int pos)
{
	result_t res = parser_impl::step(to_event(c), c, pos);
	if (state::get() == state_t::in_value)
	{
		if (result_t::s_done == res)
		{
			// TODO: this must be done through additional events as it is already done in object_parser
			state::set(state_t::after_val); // forced state change
			res = result_t::s_need_more;		// continue parsing
		}
		else 
		if (result_t::s_done_rpt == res)
		{
			// TODO: this must be done through additional events as it is already done in object_parser
			state::set(state_t::after_val); // forced state change
			res = parser_impl::step(to_event(c), c, pos);
		}
	}
	return res;
}

array_parser::event_t
array_parser::to_event(const char& c) const
{
	event_t s = event_t::symbol;

	switch (state::get())
	{
	case state_t::initial:
		if (0x5B == c)
			s = event_t::left_square_bracket;
		break;
	case state_t::before_val:
		if (0x5D == c)
			s = event_t::right_square_bracket;
		break;
	case state_t::after_val:
		switch (c)
		{
		case 0x5D:
			s = event_t::right_square_bracket;
			break;
		case 0x2C:
			s = event_t::comma;
			break;
		}
		break;
	}

	return s;
}

array_parser::event_t
array_parser::to_event(const result_t& r) const
{
	return event_t::symbol;
}