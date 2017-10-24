#include "parsers.h"

using namespace json;

number_parser::number_parser()
	: parser_impl(parser_id::parser_number)
	, m_state_table
	{
		{ read_state_t::initial,{		{ symbol_t::minus,					{ read_state_t::minus,			BIND(number_parser::on_minus)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::zero,			BIND(number_parser::on_zero)	} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_initial)	} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_dot)		} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::minus,{			{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::zero,			BIND(number_parser::on_zero)	} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_integer)	} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_dot)		} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::zero,{			{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::dot,{			{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::fractional,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::fractional,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::integer,{		{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::fractional,{	{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::exponential,{	{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::_failure_,{		{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
	}
{
}

number_parser::~number_parser()
{
}

error 
number_parser::on_initial(const char& c, const int pos)
{
	state::set(read_state_t::initial);
	return error::ok;
}

error 
number_parser::on_minus(const char& c, const int pos)
{
	state::set(read_state_t::minus);
	return error::ok;
}

error 
number_parser::on_integer(const char& c, const int pos)
{
	state::set(read_state_t::integer);
	return error::ok;
}

error 
number_parser::on_fractional(const char& c, const int pos)
{
	state::set(read_state_t::fractional);
	return error::ok;
}

error 
number_parser::on_exponential(const char& c, const int pos)
{
	state::set(read_state_t::exponential);
	return error::ok;
}

error 
number_parser::on_failure(const char& c, const int pos)
{
	state::set(read_state_t::_failure_);
	return error::fatal;
}

error 
number_parser::on_zero(const char& c, const int pos)
{
	state::set(read_state_t::zero);
	return error::ok;
}

error 
number_parser::on_dot(const char& c, const int pos)
{
	state::set(read_state_t::dot);
	return error::ok;
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