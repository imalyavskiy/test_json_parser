#include "parsers.h"

using namespace json;

array_parser::array_parser()
	: m_state_table
	{
		{ read_state_t::initial,	{	{ symbol_t::left_square_bracket,	{ read_state_t::before_val,	BIND(array_parser::on_before_value)	} },
		} },
 		{ read_state_t::before_val,	{	{ symbol_t::right_square_bracket,	{ read_state_t::done,		BIND(array_parser::on_done)			} },
 										{ symbol_t::other,					{ read_state_t::in_value,	BIND(array_parser::on_value)		} },
 		} },
		{ read_state_t::after_val,	{	{ symbol_t::right_square_bracket,	{ read_state_t::done,		BIND(array_parser::on_done)			} },
										{ symbol_t::comma,					{ read_state_t::before_val,	BIND(array_parser::on_before_value)	} },
		} },
 		{ read_state_t::in_value,	{	{ symbol_t::other,					{ read_state_t::in_value,	BIND(array_parser::on_value)		} },
 		} },
	}
	, m_value_parser(create_value_parser())
{
	reset();
}

array_parser::~array_parser()
{
}

result
array_parser::on_before_value(const unsigned char& c, const int pos)
{
	m_value_parser->reset();
	return result::s_need_more;
}

result
array_parser::on_after_value(const unsigned& c, const int pos)
{
	return result::s_need_more;
}

result 
array_parser::on_value(const unsigned& c, const int pos)
{
	return m_value_parser->step(c, pos);
}

result
array_parser::on_done(const unsigned char& c, const int pos)
{
	return result::s_done;
}

void 
array_parser::reset()
{
	state::set(read_state_t::initial);
	m_value_parser->reset();
}