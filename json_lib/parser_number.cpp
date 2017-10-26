#include "parsers.h"

using namespace json;

number_parser::number_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,			{	{ event_t::minus,					{ state_t::leading_minus,	BIND(number_parser::on_minus)		} },
										{ event_t::dec_zero,				{ state_t::zero,			BIND(number_parser::on_zero)		} },
										{ event_t::dec_digit,				{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::leading_minus,	{	{ event_t::dec_zero,				{ state_t::zero,			BIND(number_parser::on_zero)		} },
										{ event_t::dec_digit,				{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::zero,			{	{ event_t::dot,						{ state_t::dot,				BIND(number_parser::on_dot)			} },
										{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::dot,				{	{ event_t::dec_zero,				{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::dec_digit,				{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::integer,			{	{ event_t::dec_zero,				{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::dec_digit,				{ state_t::integer,			BIND(number_parser::on_integer)		} },
										{ event_t::dot,						{ state_t::dot,				BIND(number_parser::on_dot)			} },
										{ event_t::other,					{ state_t::done,			BIND(number_parser::on_done)		} },
		} },
		{ state_t::fractional,		{	{ event_t::dec_zero,				{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::dec_digit,				{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ event_t::exponent,				{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ event_t::other,					{ state_t::done,			BIND(number_parser::on_done)		} },
		} },
		{ state_t::exponent_delim,	{	{ event_t::minus,					{ state_t::exponent_sign,	BIND(number_parser::on_exponent)	} },
										{ event_t::plus,					{ state_t::exponent_sign,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_zero,				{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_digit,				{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::exponent_sign,	{	{ event_t::dec_zero,				{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_digit,				{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::exponent_val,	{	{ event_t::dec_zero,				{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::dec_digit,				{ state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ event_t::other,					{ state_t::done,			BIND(number_parser::on_done)		} },
		} },
		{ state_t::done,			{	{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
		{ state_t::failure,			{	{ event_t::other,					{ state_t::failure,			BIND(number_parser::on_fail)		} },
		} },
	}
{
}

number_parser::~number_parser()
{
}

result 
number_parser::step(const char& c, const int pos)
{
	return parser_impl::step(c, pos);
// 	if (state_t::done == state::get())
// 		return result::e_fatal; // reset requred
// 
// 	result res = parser_impl::step(c, pos);
// 	
// 	if(result::e_unexpected == res)
// 	{
// 		switch (state::get())
// 		{
// 		case state_t::integer:
// 		case state_t::fractional:
// 			res = result::s_done_rpt;
// 			state::set(state_t::done);
// 			break;
// 		case state_t::exponent_val:
// 		case state_t::initial:
// 		case state_t::exponent_delim:
// 		case state_t::exponent_sign:
// 		case state_t::leading_minus:
// 		case state_t::zero:
// 		case state_t::dot:
// 		default:
// 			res = result::e_unexpected;
// 			break;
// 		}
// 	}
// 
// 	return res;
}

result
number_parser::on_initial(const unsigned char& c, const int pos)
{
	// TODO: use symbol
	return result::s_need_more;
}

result
number_parser::on_minus(const unsigned char& c, const int pos)
{
	m_positive = false;
	return result::s_need_more;
}

result
number_parser::on_integer(const unsigned char& c, const int pos)
{
	const result res = append_digit(m_integer, c);
	return result::s_ok == res ? result::s_need_more : res;
}

result
number_parser::on_fractional(const unsigned char& c, const int pos)
{
	const result res = append_digit(m_fractional, c);
	return result::s_ok == res ? result::s_need_more : res;
}

result
number_parser::on_exponent(const unsigned char& c, const int pos)
{
	m_has_exponent = true;
	return result::s_need_more;
}

result
number_parser::on_exponent_sign(const unsigned char& c, const int pos)
{
	switch (c)
	{
	case event_t::minus:
		m_exponent_positive = false; break;
	case event_t::plus:
		m_exponent_positive = true; break;
	default:
		return result::e_fatal;
	}
	return result::s_need_more;
}

result
number_parser::on_exponent_value(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}


result
number_parser::on_zero(const unsigned char& c, const int pos)
{

	const state_t s = state::get();
	result res = result::s_ok;
	
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

	return result::s_ok == res ? result::s_need_more : res;
}

result
number_parser::on_dot(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result
number_parser::on_done(const unsigned char& c, const int pos)
{
	return result::s_done_rpt;
}

result
number_parser::on_fail(const unsigned char& c, const int pos)
{
	return result::e_unexpected;
}

number_parser::event_t
number_parser::to_event(const char& c) const
{
	if ((char)event_t::minus == c)
		return event_t::minus;
	if ((char)event_t::plus == c)
		return event_t::plus;
	if ((char)event_t::dec_zero == c)
		return event_t::dec_zero;
	if ((char)event_t::dot == c)
		return event_t::dot;
	if ((char)0x45 == c || (char)0x65 == c)
		return event_t::exponent;
	if (0x31 <= c && c <= 0x39)
		return event_t::dec_digit;

	return event_t::other;
}

void 
number_parser::reset()
{
	state::set(state_t::initial);
	
	m_positive = true;
	m_integer = 0;
	m_fractional = 0;
	m_has_exponent = false;
	m_exponent_positive = true;
	m_exponent_value = 0;
}

result 
number_parser::append_digit(int& val, const unsigned char& c)
{
	if (c < 0x30 || 0x39 < c)
	{
		assert(0);
		return result::e_fatal;
	}

	val *= 10;

	switch (c)
	{
	case 0x30: break;
	case 0x31: val += 1; break;
	case 0x32: val += 2; break;
	case 0x33: val += 3; break;
	case 0x34: val += 4; break;
	case 0x35: val += 5; break;
	case 0x36: val += 6; break;
	case 0x37: val += 7; break;
	case 0x38: val += 8; break;
	case 0x39: val += 9; break;
	}

	return result::s_ok;
}