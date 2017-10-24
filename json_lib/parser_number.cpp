#include "parsers.h"

using namespace json;

number_parser::number_parser()
	: m_state_table
	{
		{ read_state_t::initial,	{	{ symbol_t::minus,					{ read_state_t::leading_minus,	BIND(number_parser::on_minus)		} },
										{ symbol_t::dec_zero,				{ read_state_t::zero,			BIND(number_parser::on_zero)		} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_integer)		} },
		} },
		{ read_state_t::leading_minus,{	{ symbol_t::dec_zero,				{ read_state_t::zero,			BIND(number_parser::on_zero)		} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_integer)		} },
		} },
		{ read_state_t::zero,		{	{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_dot)			} },
		} },
		{ read_state_t::dot,		{	{ symbol_t::dec_zero,				{ read_state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ symbol_t::dec_digit,				{ read_state_t::fractional,		BIND(number_parser::on_fractional)	} },
		} },
		{ read_state_t::integer,	{	{ symbol_t::dec_zero,				{ read_state_t::integer,		BIND(number_parser::on_integer)		} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_integer)		} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_dot)			} },
		} },
		{ read_state_t::fractional,	{	{ symbol_t::dec_zero,				{ read_state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ symbol_t::dec_digit,				{ read_state_t::fractional,		BIND(number_parser::on_fractional)	} },
										{ symbol_t::exponent,				{ read_state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
		} },
		{ read_state_t::exponent_delim,{{ symbol_t::minus,					{ read_state_t::exponent_sign,	BIND(number_parser::on_exponent)	} },
										{ symbol_t::plus,					{ read_state_t::exponent_sign,	BIND(number_parser::on_exponent)	} },
										{ symbol_t::dec_zero,				{ read_state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
										{ symbol_t::dec_digit,				{ read_state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
		} },
		{ read_state_t::exponent_sign,{	{ symbol_t::dec_zero,				{ read_state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ symbol_t::dec_digit,				{ read_state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
		} },
		{ read_state_t::exponent_val,{	{ symbol_t::dec_zero,				{ read_state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
										{ symbol_t::dec_digit,				{ read_state_t::exponent_val,	BIND(number_parser::on_exponent)	} },
		} },

	}
{
	reset();
}

number_parser::~number_parser()
{
}

result 
number_parser::step(const char& c, const int pos)
{
	if (read_state_t::done == state::get())
		return result::e_fatal; // reset requred

	result res = parser_impl::step(c, pos);
	
	if(result::e_unexpected == res)
	{
		switch (state::get())
		{
		case read_state_t::integer:
		case read_state_t::fractional:
			res = result::s_done_rpt;
			state::set(read_state_t::done);
			break;
		case read_state_t::exponent_val:
		case read_state_t::initial:
		case read_state_t::exponent_delim:
		case read_state_t::exponent_sign:
		case read_state_t::leading_minus:
		case read_state_t::zero:
		case read_state_t::dot:
		default:
			res = result::e_unexpected;
			break;
		}
	}

	return res;
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
	case symbol_t::minus:
		m_exponent_positive = false; break;
	case symbol_t::plus:
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

	const read_state_t s = state::get();
	result res = result::s_ok;
	
	switch(s)
	{
	case read_state_t::initial:
	case read_state_t::leading_minus:
	case read_state_t::integer:
		res = append_digit(m_integer, c);
		break;
	case read_state_t::dot:
	case read_state_t::fractional:
		res = append_digit(m_fractional, c);
		break;
	case read_state_t::exponent_delim:
	case read_state_t::exponent_sign:
	case read_state_t::exponent_val:
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

number_parser::symbol_t
number_parser::token_type_of(const char& c) const
{
	if ((char)symbol_t::minus == c)
		return symbol_t::minus;
	if ((char)symbol_t::plus == c)
		return symbol_t::plus;
	if ((char)symbol_t::dec_zero == c)
		return symbol_t::dec_zero;
	if ((char)symbol_t::dot == c)
		return symbol_t::dot;
	if ((char)0x45 == c || (char)0x65 == c)
		return symbol_t::exponent;
	if (0x31 <= c && c <= 0x39)
		return symbol_t::dec_digit;

	return symbol_t::other;
}

void 
number_parser::reset()
{
	state::set(read_state_t::initial);
	
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