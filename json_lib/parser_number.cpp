#include "parsers.h"

using namespace json;

number_parser::number_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,			{	{ event_t::minus,		{ state_t::leading_minus,	BIND(number_parser::on_minus)		} },
										{ event_t::dec_zero,	{ state_t::zero,			BIND(number_parser::on_zero)		} },
										{ event_t::dec_digit,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::leading_minus,	{	{ event_t::dec_zero,	{ state_t::zero,			BIND(number_parser::on_zero)		} },
										{ event_t::dec_digit,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::zero,			{	{ event_t::dot,			{ state_t::dot,				BIND(number_parser::on_dot)			} },
										{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::dot,				{	{ event_t::dec_zero,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::dec_digit,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::integer,			{	{ event_t::dec_zero,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::dec_digit,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::dot,			{ state_t::dot,				BIND(number_parser::on_dot)			} },
										{ event_t::symbol,		{ state_t::done,			BIND(number_parser::on_done)		} },
		} },
		{ state_t::fractional,		{	{ event_t::dec_zero,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::dec_digit,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::exponent,	{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ event_t::symbol,		{ state_t::done,			BIND(number_parser::on_done)		} },
		} },
		{ state_t::exponent_delim,	{	{ event_t::minus,		{ state_t::exponent_sign,	BIND(number_parser::on_exponent)	} },
										{ event_t::plus,		{ state_t::exponent_sign,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_zero,	{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_digit,	{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::exponent_sign,	{	{ event_t::dec_zero,	{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_digit,	{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::exponent_val,	{	{ event_t::dec_zero,	{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_digit,	{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::symbol,		{ state_t::done,			BIND(number_parser::on_done)		} },
		} },
		{ state_t::done,			{	{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::failure,			{	{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
	}
{
}

number_parser::~number_parser()
{
}

result_t 
number_parser::putchar(const char& c, const int pos)
{
	return parser_impl::step(to_event(c), c, pos);
}

result_t
number_parser::on_initial(const unsigned char& c, const int pos)
{
	// TODO: use symbol
	return result_t::s_need_more;
}

result_t
number_parser::on_minus(const unsigned char& c, const int pos)
{
	m_positive = false;
	return result_t::s_need_more;
}

result_t
number_parser::on_integer(const unsigned char& c, const int pos)
{
	const result_t res = append_digit(m_integer, c);
	return result_t::s_ok == res ? result_t::s_need_more : res;
}

result_t
number_parser::on_fractional(const unsigned char& c, const int pos)
{
	const result_t res = append_digit(m_fractional, c);
	return result_t::s_ok == res ? result_t::s_need_more : res;
}

result_t
number_parser::on_exponent(const unsigned char& c, const int pos)
{
	m_has_exponent = true;
	return result_t::s_need_more;
}

result_t
number_parser::on_exponent_sign(const unsigned char& c, const int pos)
{
	switch (c)
	{
	case event_t::minus:
		m_exponent_positive = false; break;
	case event_t::plus:
		m_exponent_positive = true; break;
	default:
		return result_t::e_fatal;
	}
	return result_t::s_need_more;
}

result_t
number_parser::on_exponent_value(const unsigned char& c, const int pos)
{
	return result_t::s_need_more;
}


result_t
number_parser::on_zero(const unsigned char& c, const int pos)
{

	const state_t s = state::get();
	result_t res = result_t::s_ok;
	
	switch(s)
	{
	case state_t::initial:
	case state_t::leading_minus:
	case state_t::integer:
		res = append_digit(m_integer, c);
		break;
	case state_t::dot:
	case state_t::fractional:
		res = append_digit(m_fractional, c);
		break;
	case state_t::exponent_delim:
	case state_t::exponent_sign:
	case state_t::exponent_val:
		res = append_digit(m_exponent_value, c);
		break;
	}

	return result_t::s_ok == res ? result_t::s_need_more : res;
}

result_t
number_parser::on_dot(const unsigned char& c, const int pos)
{
	return result_t::s_need_more;
}

result_t
number_parser::on_done(const unsigned char& c, const int pos)
{
	return result_t::s_done_rpt;
}

result_t
number_parser::on_fail(const unsigned char& c, const int pos)
{
	return result_t::e_unexpected;
}

number_parser::event_t
number_parser::to_event(const char& c) const
{
	if (0x2D == c)
		return event_t::minus;
	if (0x2B == c)
		return event_t::plus;
	if (0x30 == c)
		return event_t::dec_zero;
	if (0x2E == c)
		return event_t::dot;
	if (0x45 == c || 0x65 == c)
		return event_t::exponent;
	if (0x31 <= c && c <= 0x39)
		return event_t::dec_digit;

	return event_t::symbol;
}

number_parser::event_t 
number_parser::to_event(const result_t& c) const
{
	return event_t::symbol;
}

void 
number_parser::reset()
{
	state::set(state_t::initial);
	
	m_positive			= true;
	m_integer			= 0;
	m_fractional		= 0;
	m_has_exponent		= false;
	m_exponent_positive = true;
	m_exponent_value	= 0;
}

result_t 
number_parser::append_digit(int& val, const unsigned char& c)
{
	if (c < 0x30 || 0x39 < c)
		return result_t::e_fatal;

	val *= 10;

	switch (c)
	{
	case 0x31: val += 1; break; //1
	case 0x32: val += 2; break; //2
	case 0x33: val += 3; break; //3
	case 0x34: val += 4; break;	//4
	case 0x35: val += 5; break;	//5
	case 0x36: val += 6; break;	//6
	case 0x37: val += 7; break;	//7
	case 0x38: val += 8; break;	//8
	case 0x39: val += 9; break;	//9
	}

	return result_t::s_ok;
}